#pragma once
#include <memory>
#include <Core/Modules/IModule.h>
#include <Core/Modules/EngineContext.h>
#include <RHI/Module.h>
#include <Core/Log.h>
#include <Core/ECS/Scene.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"

namespace Squid {
namespace Renderer {

    using Core::EngineContext;
    using Core::IModule;

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec3 camera_pos;
        float padding0;
    };

    static_assert(sizeof(UniformBufferObject) == 3 * 64 + 16, "");

    class Module : public IModule {
    public:
        Module(EngineContext *ctx) : IModule(ctx) {}
        ~Module() override;

        bool Initialize() override;
        void Event() override;
        void Tick(float delta) override;

        void CreateRenderer(void *win);

        inline RHI::SwapchainHandle &GetSwapchain() { return this->swapchain; }
        inline void *GetWindow() const { return this->win; }
        inline std::unique_ptr<RHI::Device> &GetDevice() { return this->device; }
        inline RHI::TextureHandle &GetFrame() { return this->frame_composition; }

        inline Core::SceneGraph *GetScene() { return &scene; };
        inline Core::Entity GetSceneRoot() { return root; };

        // Viewport dimensions
        inline u32 GetFrameHeight() const { return this->frame_height; }
        inline u32 GetFrameWidth() const { return this->frame_width; }

        inline void SetFrameSize(u32 height, u32 width) {
            this->frame_height = height;
            this->frame_width = width;
           // resize = true;
        }

        bool resize = false;
        void ResizeRenderTargets();

    private:
        void UpdateUBO();
        void CreateRenderTargets();

        u32 frame_height = 2000;
        u32 frame_width = 2000;

        std::shared_ptr<RHI::Module> rhi;
        std::unique_ptr<RHI::Device> device;

        void *win = nullptr;
        RHI::SwapchainHandle swapchain;

        // Frame render resources
        std::unique_ptr<Mesh> mesh;

        std::vector<RHI::DescriptorSetHandle> descriptor_set_handles;
        RHI::GraphicsPipelineHandle gfx_pipe;
        RHI::BufferHandle ubo_handle;

        RHI::TextureHandle frame_composition;
        RHI::TextureHandle frame_ds;
        RHI::RenderPassHandle composition_pass;

        // ECS
        Core::SceneGraph scene;
        Core::Entity root;
        Core::Entity model;
        Core::TransformComponent t;
    };

} // namespace Renderer
} // namespace Squid