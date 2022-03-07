#include <array>

#include <Renderer/Module.h>
#include <Renderer/TextureImporter.h>

#include <Core/Log.h>
#include <Core/Profiling.h>
#include <Core/FileSystem.h>
#include <EditorCore/Module.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Squid {
namespace Renderer {

    Module::~Module() {
        // device->UnloadHandle(swapchain);
        device.reset();
    };

    bool Module::Initialize() {
        rhi = context->GetModule<RHI::Module>("RHI");

        scene = Core::SceneGraph();

        root = Core::CreateEntity();
        model = Core::CreateEntity();

        auto model2 = Core::CreateEntity();
        auto model3 = Core::CreateEntity();

        auto rt = scene.transforms.Create(root);
        auto mt = scene.transforms.Create(model);
        auto mt2 = scene.transforms.Create(model2);
        auto mt3 = scene.transforms.Create(model3);

        scene.transforms.GetComponent(root)->name = std::string("Root");
        scene.transforms.GetComponent(model)->name = std::string("Model");
        scene.transforms.GetComponent(model2)->name = std::string("Model2");
        scene.transforms.GetComponent(model3)->name = std::string("Model3");

        scene.Attach(model, root);
        scene.Attach(model2, model);
        scene.Attach(model3, root);

        return true;
    }

    void Module::UpdateUBO() {
        static auto start_time = std::chrono::high_resolution_clock::now();

        auto current_time = std::chrono::high_resolution_clock::now();
        float time =
            std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count() * 0.1;

        UniformBufferObject ubo = {};

        auto t = scene.transforms.GetComponent(model);
        // t->Scale(glm::vec3(0, 0, 0));
        t->scale_local = glm::vec3(15.0, 15.0, 15.0);
        t->Rotate(glm::vec3(0.0f, 0.00, 0.001));
        t->UpdateTransform();

        // ubo.model = scene.transforms.GetComponent(model)->world;
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(2.0, 2.0, 2.0));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), (float)frame_height / (float)frame_width, 0.1f, 1000.0f);
        ubo.proj[1][1] *= -1;
        ubo.camera_pos = glm::vec3(2.0f, 2.0f, 2.0f);

        void *ubo_data = device->MapBuffer(ubo_handle);
        memcpy(ubo_data, &ubo, sizeof(ubo));
        device->UnmapBuffer(ubo_handle);
    }

    void Module::ResizeRenderTargets() {
        device->UnloadTexture(frame_composition);

        device->LoadTexture(frame_composition);
    }

    void Module::CreateRenderer(void *win) {
        this->win = win;

        auto adapters = rhi->EnumerateAdapters();
        auto selected = std::find_if(adapters.begin(), adapters.end(), [](const auto &adapter) {
            return adapter->info.type == RHI::AdapterType::DEDICATED;
        });

        this->device = selected->get()->CreateDevice();

        swapchain.backbuffer = {};
        swapchain.backbuffer._offscreen = false;
        swapchain.window_handle = win;
        device->LoadSwapchain(swapchain);

        RHI::CommandList transfer_list = device->BeginTransferList();

        TextureImporter importer = TextureImporter(device.get(), transfer_list);

        std::array<const std::string, 6> env_files = {"Assets/Textures/px_1k.hdr", "Assets/Textures/nx_1k.hdr",
                                                      "Assets/Textures/py_1k.hdr", "Assets/Textures/ny_1k.hdr",
                                                      "Assets/Textures/pz_1k.hdr", "Assets/Textures/nz_1k.hdr"};

        RHI::TextureHandle env_map = importer.FromEnvFile(env_files, "Env map");

        RHI::TextureHandle glock_albedo = importer.FromFile(
            "Assets/Textures/Glock_01_Albedo.png", "Glock Albedo", RHI::FORMAT_R8G8B8A8_UNORM_SRGB); // sRGB space

        RHI::TextureHandle glock_normal = importer.FromFile(
            "Assets/Textures/Glock_01_Normal.png", "Glock Normal", RHI::FORMAT_R8G8B8A8_UNORM); // Linear space

        importer.Upload();

        // Frame target
        frame_composition.height = frame_height;
        frame_composition.width = frame_width;
        frame_composition.depth = 1;
        frame_composition.format = RHI::FORMAT_B8G8R8A8_UNORM;
        frame_composition.mip_levels = 1;
        frame_composition.sample_count = 1;
        frame_composition.usage_flags =
            RHI::TextureHandle::Usage::SHADER_RESOURCE_VIEW | RHI::TextureHandle::Usage::RENDER_TARGET_VIEW;
        device->LoadTexture(frame_composition);
        frame_composition.layout = RHI::ImageLayout::UNDEFINED;
        device->SetName(frame_composition, "Frame Target");

        frame_ds.height = frame_height;
        frame_ds.width = frame_width;
        frame_ds.depth = 1;
        frame_ds.format = RHI::FORMAT_D24_UNORM_S8_UINT;
        frame_ds.mip_levels = 1;
        frame_ds.sample_count = 1;
        frame_ds.layout = ImageLayout::UNDEFINED;
        frame_ds.usage_flags = RHI::TextureHandle::Usage::DEPTH_STENCIL_VIEW;
        device->LoadTexture(frame_ds);
        device->SetName(frame_ds, "Frame Target DS");

        composition_pass.num_attachments = 1;
        composition_pass.color[0] = {RHI::RenderPassAttachment::COLOR_ATTACHMENT,
                                     RHI::RenderPassAttachment::LOAD_OP_CLEAR,
                                     &frame_composition,
                                     RHI::RenderPassAttachment::STORE_OP_STORE,
                                     RHI::ImageLayout::UNDEFINED,
                                     RHI::ImageLayout::SHADER_RESOURCE};

        composition_pass.ds = {RHI::RenderPassAttachment::DEPTH_STENCIL_ATTACHMENT,
                               RHI::RenderPassAttachment::LOAD_OP_CLEAR,
                               &frame_ds,
                               RHI::RenderPassAttachment::STORE_OP_STORE,
                               RHI::ImageLayout::UNDEFINED,
                               RHI::ImageLayout::GENERAL};

        device->LoadRenderPass(composition_pass);
        device->SetName(composition_pass, "Frame Target Render Pass");

        ubo_handle.cpu_access = true;
        ubo_handle.size = sizeof(UniformBufferObject);
        ubo_handle.usage = RHI::BufferHandle::Usage::UNIFORM_BUFFER;
        device->LoadBuffer(ubo_handle);
        device->SetName(ubo_handle, "Frame Target UBO");

        // Descriptors
        RHI::Descriptor ubo_descriptor;
        ubo_descriptor.type = RHI::Descriptor::Type::Uniform;
        ubo_descriptor.shader_stage = RHI::SHADER_STAGE_VERTEX_STAGE | RHI::SHADER_STAGE_PIXEL_STAGE;
        ubo_descriptor.count = 1;
        ubo_descriptor.binding = 0;

        RHI::Descriptor env_desc;
        env_desc.type = RHI::Descriptor::Type::Sampler;
        env_desc.shader_stage = RHI::SHADER_STAGE_PIXEL_STAGE;
        env_desc.count = 1;
        env_desc.binding = 1;

        RHI::Descriptor sampler_desc1;
        sampler_desc1.type = RHI::Descriptor::Type::Sampler;
        sampler_desc1.shader_stage = RHI::SHADER_STAGE_PIXEL_STAGE;
        sampler_desc1.count = 1;
        sampler_desc1.binding = 2;

        RHI::Descriptor sampler_desc2;
        sampler_desc2.type = RHI::Descriptor::Type::Sampler;
        sampler_desc2.shader_stage = RHI::SHADER_STAGE_PIXEL_STAGE;
        sampler_desc2.count = 1;
        sampler_desc2.binding = 3;

        RHI::DescriptorSetHandle descriptor_set_handle;
        descriptor_set_handle.descriptors = {ubo_descriptor, env_desc, sampler_desc1, sampler_desc2};
        device->LoadDescriptorSet(descriptor_set_handle);
        descriptor_set_handles.push_back(descriptor_set_handle);

        device->BindBuffer(descriptor_set_handle, 0, ubo_handle);
        device->BindTexture(descriptor_set_handle, 1, env_map);
        device->BindTexture(descriptor_set_handle, 2, glock_albedo);
        device->BindTexture(descriptor_set_handle, 3, glock_normal);
        this->UpdateUBO();

        gfx_pipe.cull_mode = RHI::CullMode::FRONT;
        gfx_pipe.depth_test = true;
        gfx_pipe.stencil_test = false;
        gfx_pipe.depth_write = true;
        gfx_pipe.compare_op = RHI::CompareOp::LESS;
        gfx_pipe.render_pass = &composition_pass;
        gfx_pipe.vertex_layout.stride = sizeof(MeshVertex);
        gfx_pipe.vertex_layout.input_count = 4;
        gfx_pipe.vertex_layout.inputs[0].binding = 0;
        gfx_pipe.vertex_layout.inputs[0].type = RHI::VertexType::FLOAT3;
        gfx_pipe.vertex_layout.inputs[0].offset = offsetof(MeshVertex, position);
        gfx_pipe.vertex_layout.inputs[1].binding = 1;
        gfx_pipe.vertex_layout.inputs[1].type = RHI::VertexType::FLOAT3;
        gfx_pipe.vertex_layout.inputs[1].offset = offsetof(MeshVertex, normal);
        gfx_pipe.vertex_layout.inputs[2].binding = 2;
        gfx_pipe.vertex_layout.inputs[2].type = RHI::VertexType::FLOAT3;
        gfx_pipe.vertex_layout.inputs[2].offset = offsetof(MeshVertex, color);
        gfx_pipe.vertex_layout.inputs[3].binding = 3;
        gfx_pipe.vertex_layout.inputs[3].type = RHI::VertexType::FLOAT2;
        gfx_pipe.vertex_layout.inputs[3].offset = offsetof(MeshVertex, uv0);
        gfx_pipe.vertex_shader = Core::ReadTextFile("Assets/Shaders/unlit.vert.spv");
        gfx_pipe.pixel_shader = Core::ReadTextFile("Assets/Shaders/unlit.frag.spv");
        gfx_pipe.descriptor_sets = descriptor_set_handles;
        gfx_pipe.primitive_restart = false;
        device->LoadPipeline(gfx_pipe);

        mesh = std::make_unique<Mesh>("Assets/Models/Glock_01.obj");
        mesh->LoadOnDevice(device);
    }

    void Module::Event() {}

    void Module::Tick(float delta) {
        PROFILING_SCOPE

        {
            PROFILING_NAMED_SCOPE("Upadate Renderer UBO")
            UpdateUBO();
        }

        // Main frame render
        auto list = device->BeginCommandListEXP();
        device->BeginRenderPassEXP(list, composition_pass);
        // Draw stuff
        RHI::Viewport vp;
        vp.height = 1000;
        vp.width = 1000;
        vp.x = 0;
        vp.y = 0;
        vp.min_depth = 0.0f;
        vp.max_depth = 1.0f;

        LOG("{} {}", this->frame_height, this->frame_width)

        RHI::Rect sc;
        sc.height = 1000;
        sc.width = 1000;
        sc.x = 0;
        sc.y = 0;

        device->BindVertexBuffer(list, mesh->GetVertexBuffer(), 0);
        device->BindIndexBuffer(list, mesh->GetIndexBuffer(), 0, RHI::IndexFormat::INDEX_32BIT);
        device->BindPipelineState(list, gfx_pipe);
        device->BindViewports(list, 1, &vp);
        device->BindScissorRects(list, 1, &sc);
        device->BindDescriptorSet(list, gfx_pipe, descriptor_set_handles[0], 0);
        device->DrawIndexed(list, mesh->GetVerticesCount(), 0, 0);

        device->EndRenderPass(list);
    }

    IMPLEMENT_MODULE(Module, Renderer)
} // namespace Renderer
} // namespace Squid
