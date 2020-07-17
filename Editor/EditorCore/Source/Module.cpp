#include <Public/EditorCore/Module.h>
#include "Widgets/LogWidget.h"
#include "Editor.h"

#include "ImGui/imgui_impl_rhi.h"
#include "ImGui/imgui_impl_sdl.h"
#include <imgui.h>


namespace Squid {
namespace EditorCore {
    bool Module::Initialize() {
        // std::cout.rdbuf(buffer.rdbuf());
        rhi = context->GetModule<RHI::Module>("RHI");
        renderer = context->GetModule<Renderer::Module>("Renderer");

        const auto window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ctx = ImGui::CreateContext();
        ImGui::SetCurrentContext(ctx);

        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui_ImplSDL2_InitForVulkan((SDL_Window *)renderer->GetWindow());
        ImGui_ImplRHI_Init(renderer->GetDevice().get());

        this->editor = new Editor(renderer);
        return true;
    };

    void Module::Tick(float delta) {
        auto& device = renderer->GetDevice();

        

        

        ImGuiIO &io = ImGui::GetIO();
        (void)io;

        ImGui_ImplRHI_NewFrame();
        ImGui_ImplSDL2_NewFrame((SDL_Window *)renderer->GetWindow());

        ImGui::NewFrame();
        editor->Tick();
        ImGui::Render();

        RHI::CommandList list = device->BeginCommandListEXP();
        g_device->BeginRenderPass(list, renderer->GetSwapchain().backbuffer);
        ImGui_ImplRHI_RenderDrawData(ImGui::GetDrawData(), list);
        g_device->EndRenderPass(list);

        // renderer->GetDevice();
    };

    // NOTE disabled support for dynamic link
    // IMPLEMENT_MODULE(Module, EditorCore)

} // namespace EditorCore
}; // namespace Squid
