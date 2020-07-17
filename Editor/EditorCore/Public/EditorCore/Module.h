#pragma once
#include <Core/Modules/IModule.h>
#include <Core/Modules/EngineContext.h>
#include <Core/Log.h>

#include <Renderer/Module.h>
#include <imgui.h>
#include <sstream>

namespace Squid {
namespace EditorCore {

    using Core::EngineContext;
    using Core::IModule;

    class Editor;

    class Module : public IModule {
    public:
        Module(EngineContext* ctx) : IModule(ctx) {}
        bool Initialize() override;
        void Tick(float delta) override;

        // TODO smart pointer
        ~Module() { delete editor; }

        ImGuiContext *GetContext() const { return ctx; };

    private:
        std::shared_ptr<RHI::Module> rhi;
        std::shared_ptr<Renderer::Module> renderer;

        ImGuiContext *ctx = nullptr;
        Editor *editor;
        std::stringstream buffer;
    };

} // namespace EditorCore
} // namespace Squid