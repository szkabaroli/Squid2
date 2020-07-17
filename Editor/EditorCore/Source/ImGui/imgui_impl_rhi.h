#pragma once

#include <RHI/Module.h>
#include <Core/FileSystem.h>
#include <vector>
#include <unordered_map>
#include <fstream>

using namespace Squid;

static bool g_initialized = false;
static RHI::Device *g_device;

static RHI::TextureHandle g_fonts_texture;
static RHI::BufferHandle g_ubo;
static RHI::DescriptorSetHandle g_set;
static std::unordered_map<u32, RHI::DescriptorSetHandle> g_texture_sets;

static RHI::GraphicsPipelineHandle g_pso;

static RHI::BufferHandle g_vb;
static RHI::BufferHandle g_ib;

static int g_vb_size = 5000;
static int g_ib_size = 10000;

struct UniformBufferObject {
    float scale[2];
    float translate[2];
};

// Forward declarataions
void ImGui_ImplRHI_InitPlatformInterface();
void ImGui_ImplRHI_ShutdownPlatformInterface();

bool ImGui_ImplRHI_Init(RHI::Device *device) {
    // Setup back-end capabilities flags
    ImGuiIO &io = ImGui::GetIO();
    g_device = device;

    io.BackendRendererName = "imgui_impl_rhi";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field,
                                                               // allowing for large meshes.
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports; // We can create multi-viewports on
                                                               // the Renderer side (optional)

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ImGui_ImplRHI_InitPlatformInterface();

    g_vb = RHI::BufferHandle();
    g_ib = RHI::BufferHandle();

    g_pso = RHI::GraphicsPipelineHandle();

    g_fonts_texture = RHI::TextureHandle();
    g_set = RHI::DescriptorSetHandle();
    g_ubo = RHI::BufferHandle();

    return true;
}

void ImGui_ImplRHI_Shutdown() {
    ImGui_ImplRHI_ShutdownPlatformInterface();

    g_device->UnloadPipeline(g_pso);
    g_device->UnloadDescriptorSet(g_set);
    g_device->UnloadTexture(g_fonts_texture);
    g_device->UnloadBuffer(g_ubo);
    g_device->UnloadBuffer(g_vb);
    g_device->UnloadBuffer(g_ib);

    // ImGui_ImplRHI_InvalidateDeviceObjects();
    g_device = nullptr;
}

static void ImGui_ImplRHI_CreateFontsTexture() {
    // Build texture atlas
    ImGuiIO &io = ImGui::GetIO();
    unsigned char *pixels;
    int tex_width, tex_height, bytes_pp;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &tex_width, &tex_height, &bytes_pp);

    uint64_t image_size = tex_width * tex_height * bytes_pp;

    RHI::BufferHandle staging_fonts_texture = {};
    staging_fonts_texture.cpu_access = true;
    staging_fonts_texture.size = image_size;
    staging_fonts_texture.usage = RHI::BufferHandle::Usage::TRANSFER_SRC;
    g_device->LoadBuffer(staging_fonts_texture);
    g_device->SetName(staging_fonts_texture, "ImGui Staging Fonts Buffer");

    void *texture_data = g_device->MapBuffer(staging_fonts_texture);
    memcpy(texture_data, pixels, image_size);
    g_device->UnmapBuffer(staging_fonts_texture);

    g_fonts_texture = {};
    g_fonts_texture.height = tex_height;
    g_fonts_texture.width = tex_width;
    g_fonts_texture.depth = 1;
    g_fonts_texture.size = image_size;
    g_fonts_texture.format = RHI::FORMAT_R8G8B8A8_UNORM_SRGB;
    g_fonts_texture.mip_levels = 1;
    g_fonts_texture.sample_count = 1;
    g_fonts_texture.usage_flags = RHI::TextureHandle::Usage::SHADER_RESOURCE_VIEW;
    g_device->LoadTexture(g_fonts_texture);
    g_device->SetName(g_fonts_texture, "ImGui Fonts Texture");

    RHI::CommandList transfer_list = g_device->BeginTransferList();
    g_device->Copy(transfer_list, staging_fonts_texture, g_fonts_texture);
    g_device->QueueSubmit(RHI::QueueType::GRAPHICS, transfer_list);

    g_device->UnloadBuffer(staging_fonts_texture);

    io.Fonts->TexID = (ImTextureID)&g_fonts_texture;
}

void CreateDescriptorSetForTexture(const RHI::TextureHandle &handle) {
    auto el = g_texture_sets.find(handle.id);
    if (el == g_texture_sets.end()) {

        RHI::Descriptor texture_descriptor;
        texture_descriptor.binding = 0;
        texture_descriptor.count = 1;
        texture_descriptor.shader_stage = RHI::SHADER_STAGE_PIXEL_STAGE;
        texture_descriptor.type = RHI::Descriptor::Type::Sampler;

        RHI::DescriptorSetHandle set;
        set.descriptors = {texture_descriptor};
        g_device->LoadDescriptorSet(set);
        g_device->BindTexture(set, 0, handle);

        g_texture_sets.insert(std::pair(handle.id, set));
    }
}

// Create RHI Resources
void ImGui_ImplRHI_CreateDeviceObjects() {
    // Create the constant buffer
    g_ubo = {};
    g_ubo.usage = RHI::BufferHandle::Usage::UNIFORM_BUFFER;
    g_ubo.size = sizeof(UniformBufferObject);
    g_ubo.cpu_access = true;
    g_device->LoadBuffer(g_ubo);
    g_device->SetName(g_ubo, "ImGui UBO");

    ImGui_ImplRHI_CreateFontsTexture();

    RHI::Descriptor ubo_descriptor;
    ubo_descriptor.binding = 0;
    ubo_descriptor.count = 1;
    ubo_descriptor.shader_stage = RHI::SHADER_STAGE_VERTEX_STAGE;
    ubo_descriptor.type = RHI::Descriptor::Type::Uniform;

    RHI::Descriptor texture_descriptor;
    texture_descriptor.binding = 1;
    texture_descriptor.count = 1;
    texture_descriptor.shader_stage = RHI::SHADER_STAGE_PIXEL_STAGE;
    texture_descriptor.type = RHI::Descriptor::Type::Sampler;

    g_set.descriptors = {ubo_descriptor};
    g_device->LoadDescriptorSet(g_set);
    g_device->BindBuffer(g_set, 0, g_ubo);

    CreateDescriptorSetForTexture(g_fonts_texture);

    std::vector<RHI::DescriptorSetHandle> sets{g_set, g_texture_sets.begin()->second};

    // Vertex Layout
    g_pso.vertex_layout.stride = sizeof(ImDrawVert);
    g_pso.vertex_layout.input_count = 3;
    g_pso.vertex_layout.inputs[0].type = RHI::VertexType::FLOAT2;
    g_pso.vertex_layout.inputs[0].offset = offsetof(ImDrawVert, pos);
    g_pso.vertex_layout.inputs[0].binding = 0;
    g_pso.vertex_layout.inputs[1].type = RHI::VertexType::FLOAT2;
    g_pso.vertex_layout.inputs[1].offset = offsetof(ImDrawVert, uv);
    g_pso.vertex_layout.inputs[1].binding = 1;
    g_pso.vertex_layout.inputs[2].type = RHI::VertexType::FLOAT4;
    g_pso.vertex_layout.inputs[2].offset = offsetof(ImDrawVert, col);
    g_pso.vertex_layout.inputs[2].binding = 2;

    // Blending
    g_pso.blend_state.enabled = true;
    g_pso.blend_state.rt_blends[0].src_blend = RHI::Blend::SRC_ALPHA;
    g_pso.blend_state.rt_blends[0].dst_blend = RHI::Blend::INV_SRC_ALPHA;
    g_pso.blend_state.rt_blends[0].src_blend_alpha = RHI::Blend::INV_SRC_ALPHA;
    g_pso.blend_state.rt_blends[0].dst_blend_alpha = RHI::Blend::ZERO;

    g_pso.cull_mode = RHI::CullMode::NONE;
    g_pso.descriptor_sets = sets;
    g_pso.vertex_shader = Squid::Core::ReadTextFile("Assets/Shaders/imgui.vert.spv");
    g_pso.pixel_shader = Squid::Core::ReadTextFile("Assets/Shaders/imgui.frag.spv");
    g_pso.primitive_restart = false;
    g_pso.stencil_test = true;
    g_pso.depth_test = true;
    g_pso.depth_write = true;
    g_device->LoadPipeline(g_pso);

    g_initialized = true;
}

// Render UI from imgui data
void ImGui_ImplRHI_RenderDrawData(ImDrawData *draw_data, const RHI::CommandList &list) {
    // Avoid rendering when minimized
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    // Create or grow vertex/index buffers if needed
    if (!g_device->HasBuffer(g_vb) || g_vb_size < draw_data->TotalVtxCount) {
        if (g_device->HasBuffer(g_vb))
            g_device->UnloadBuffer(g_vb);

        g_vb = {};
        g_vb_size = draw_data->TotalVtxCount + 5000;
        g_vb.cpu_access = true;
        g_vb.size = g_vb_size * sizeof(ImDrawVert);
        g_vb.usage = RHI::BufferHandle::Usage::VERTEX_BUFFER;

        g_device->LoadBuffer(g_vb);
        g_device->SetName(g_vb, "ImGui Vertex Buffer");
    }

    if (!g_device->HasBuffer(g_ib) || g_ib_size < draw_data->TotalIdxCount) {
        if (g_device->HasBuffer(g_ib))
            g_device->UnloadBuffer(g_ib);

        g_ib = {};
        g_ib_size = draw_data->TotalIdxCount + 10000;
        g_ib.cpu_access = true;
        g_ib.size = g_ib_size * sizeof(ImDrawIdx);
        g_ib.usage = RHI::BufferHandle::Usage::INDEX_BUFFER;

        g_device->LoadBuffer(g_ib);
        g_device->SetName(g_vb, "ImGui Index Buffer");
    }

    // Upload vertex/index data into a single contiguous GPU buffer

    ImDrawVert *vtx_dst = (ImDrawVert *)g_device->MapBuffer(g_vb);
    ImDrawIdx *idx_dst = (ImDrawIdx *)g_device->MapBuffer(g_ib);

    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList *cmd_list = draw_data->CmdLists[n];
        memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtx_dst += cmd_list->VtxBuffer.Size;
        idx_dst += cmd_list->IdxBuffer.Size;
    }

    g_device->UnmapBuffer(g_vb);
    g_device->UnmapBuffer(g_ib);

    // Setup orthographic projection matrix into our constant buffer
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to
    // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single
    // viewport apps.

    {
        UniformBufferObject *ubo_data = (UniformBufferObject *)g_device->MapBuffer(g_ubo);

        float scale[2];
        scale[0] = 2.0f / draw_data->DisplaySize.x;
        scale[1] = 2.0f / draw_data->DisplaySize.y;
        float translate[2];
        translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0];
        translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];

        memcpy(&ubo_data->scale, scale, sizeof(scale));
        memcpy(&ubo_data->translate, translate, sizeof(translate));

        g_device->UnmapBuffer(g_ubo);
    }

    g_device->BindIndexBuffer(list, g_ib, 0);
    g_device->BindVertexBuffer(list, g_vb, 0);

    ImVec2 display_size = draw_data->DisplaySize;
    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    RHI::Viewport viewport;
    viewport.width = (uint32_t)display_size.x;
    viewport.height = (uint32_t)display_size.y;

    g_device->BindDescriptorSet(list, g_pso, g_set, 0);
    g_device->BindViewports(list, 1, &viewport);
    g_device->BindPipelineState(list, g_pso);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_idx_offset = 0;
    int global_vtx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++) {

        const ImDrawList *cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];

            // TODO user callback
            // Apply scissor/clipping rectangle
            ImVec4 clip_rect;
            clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
            clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
            clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
            clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

            if (clip_rect.x < display_size.x && clip_rect.y < display_size.y && clip_rect.z >= 0.0f &&
                clip_rect.w >= 0.0f) {
                // Negative offsets are illegal for vkCmdSetScissor
                if (clip_rect.x < 0.0f)
                    clip_rect.x = 0.0f;
                if (clip_rect.y < 0.0f)
                    clip_rect.y = 0.0f;

                // Apply scissor/clipping rectangle
                RHI::Rect scissor;
                scissor.x = (int32_t)(clip_rect.x);
                scissor.y = (int32_t)(clip_rect.y);
                scissor.width = (uint32_t)(clip_rect.z - clip_rect.x);
                scissor.height = (uint32_t)(clip_rect.w - clip_rect.y);

                g_device->BindScissorRects(list, 1, &scissor);
                auto tex = *(RHI::TextureHandle *)pcmd->TextureId;
                // g_device->BindTexture(g_set, 1, tex);
                // g_device->BindDescriptorSet(list, g_pso, g_set);

                // Id no set for texture create one
                auto el = g_texture_sets.find(tex.id);
                if (el == g_texture_sets.end()) {
                    CreateDescriptorSetForTexture(tex);
                } else {

                    // g_device->BindTexture(el->second, 0, tex);
                }

                g_device->BindDescriptorSet(list, g_pso, g_texture_sets[tex.id], 1);

                // Bind texture, Draw
                // std::vector<RHI::DescriptorSetHandle> sets = {g_set};

                g_device->DrawIndexed(
                    list, pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // g_device->PrerecordList(list);
    // g_device->QueueSubmit(RHI::QueueType::GRAPHICS, list);
}

// Create Fonts texture atlas
void ImGui_ImplRHI_NewFrame() {
    if (!g_initialized)
        ImGui_ImplRHI_CreateDeviceObjects();
}

struct ImGuiViewportDataRHI {
    RHI::RenderTargetHandle backbuffer_handle;
    RHI::SwapchainHandle swapchain_handle;
    RHI::CommandList list;

    ImGuiViewportDataRHI() {
        swapchain_handle = {};
        backbuffer_handle = {};
        list = {};
    }

    ~ImGuiViewportDataRHI() {}
};

static void ImGui_ImplRHI_CreateWindow(ImGuiViewport *viewport) {
    ImGuiViewportDataRHI *data = IM_NEW(ImGuiViewportDataRHI)();
    viewport->RendererUserData = data;

    // PlatformHandleRaw should always be a HWND, whereas PlatformHandle might be a higher-level
    // handle (e.g. GLFWWindow*, SDL_Window*). Some back-end will leave PlatformHandleRaw NULL, in
    // which case we assume PlatformHandle will contain the HWND.
    void *win = viewport->PlatformHandle;
    assert(win != nullptr);

    // Backbuffer Handle
    data->backbuffer_handle._offscreen = false;

    // Swapchain Handle
    data->swapchain_handle.window_handle = win;
    data->swapchain_handle.backbuffer = data->backbuffer_handle;

    g_device->LoadSwapchain(data->swapchain_handle);
}

static void ImGui_ImplRHI_DestroyWindow(ImGuiViewport *viewport) {
    // The main viewport (owned by the application) will always have RendererUserData == NULL since
    // we didn't create the data for it.
    if (ImGuiViewportDataRHI *data = (ImGuiViewportDataRHI *)viewport->RendererUserData) {
        // if (g_device->IsLoaded(data->swapchain_handle))
        //    g_device->UnloadHandle(data->swapchain_handle);

        IM_DELETE(data);
    }
    viewport->RendererUserData = nullptr;
}

static void ImGui_ImplRHI_SetWindowSize(ImGuiViewport *viewport, ImVec2 size) {
    ImGuiViewportDataRHI *data = (ImGuiViewportDataRHI *)viewport->RendererUserData;

    /*if (data->RTView) {
        data->RTView->Release();
        data->RTView = NULL;
    }

    if (data->SwapChain) {
        ID3D11Texture2D *pBackBuffer = NULL;
        data->SwapChain->ResizeBuffers(0, (UINT)size.x, (UINT)size.y, DXGI_FORMAT_UNKNOWN, 0);
        data->SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (pBackBuffer == NULL) {
            fprintf(stderr, "ImGui_ImplDX11_SetWindowSize() failed creating buffers.\n");
            return;
        }
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &data->RTView);
        pBackBuffer->Release();
    }*/
}

static void ImGui_ImplRHI_RenderWindow(ImGuiViewport *viewport, void *) {
    ImGuiViewportDataRHI *data = (ImGuiViewportDataRHI *)viewport->RendererUserData;

    g_device->BeginFrameEXP(data->swapchain_handle);

    data->list = g_device->BeginCommandListEXP();
    g_device->BeginRenderPass(data->list, data->backbuffer_handle);

    ImGui_ImplRHI_RenderDrawData(viewport->DrawData, data->list);
}

static void ImGui_ImplRHI_SwapBuffers(ImGuiViewport *viewport, void *) {
    ImGuiViewportDataRHI *data = (ImGuiViewportDataRHI *)viewport->RendererUserData;
    g_device->EndRenderPass(data->list);
    g_device->EndFrameEXP(data->swapchain_handle);
}

static void ImGui_ImplRHI_InitPlatformInterface() {
    ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_CreateWindow = ImGui_ImplRHI_CreateWindow;
    platform_io.Renderer_DestroyWindow = ImGui_ImplRHI_DestroyWindow;
    platform_io.Renderer_SetWindowSize = ImGui_ImplRHI_SetWindowSize;
    platform_io.Renderer_RenderWindow = ImGui_ImplRHI_RenderWindow;
    platform_io.Renderer_SwapBuffers = ImGui_ImplRHI_SwapBuffers;
}

static void ImGui_ImplRHI_ShutdownPlatformInterface() { ImGui::DestroyPlatformWindows(); }