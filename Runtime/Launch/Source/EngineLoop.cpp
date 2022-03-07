#include <pch.h>
#include "EngineLoop.h"
#include <Core/ECS/Scene.h>
#include <Core/Profiling.h>
#include <Core/Modules/ModuleManager.h>
#include <Core/Modules/EngineContext.h>

#include <RHI/Module.h>
#include <Renderer/Module.h>

#include <EditorCore/Module.h>

namespace Squid {

EngineLoop::EngineLoop() {}

EngineLoop::~EngineLoop() {}

int EngineLoop::Start() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Failed to initialize the SDL2 library");

    auto window = SDL_CreateWindow(
        "Squid Editor RTX(ON)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    auto manager = Core::ModuleManager::GetInstance();
    Core::EngineContext ctx = {};

    auto rhi = manager.RegisterModule<RHI::Module>(&ctx, "VulkanRHI").value();
    auto renderer = std::make_shared<Renderer::Module>(&ctx);
#ifdef SQUID_EDITOR
    auto editor = std::make_shared<EditorCore::Module>(&ctx);
#endif

    ctx.SetModule(rhi, "RHI");
    ctx.SetModule(renderer, "Renderer");
    ctx.SetModule(editor, "Editor");

    rhi->Initialize();
    renderer->Initialize();

    // Create RHI Instance

    renderer->CreateRenderer(window);

    RHI::TextureHandle rt = {};
    rt.format = RHI::Format::FORMAT_R8G8B8A8_UNORM;
    rt.usage_flags = RHI::TextureHandle::Usage::RENDER_TARGET_VIEW | RHI::TextureHandle::Usage::SHADER_RESOURCE_VIEW;
    rt.width = 1920;
    rt.height = 1080;
    // renderer->GetDevice()->LoadTexture(rt);

    // RHI::RenderPassHandle cube_pass = {};
    // cube_pass.num_attachments = 1;
    // cube_pass.attachments[0] = {RHI::RenderPassAttachment::COLOR, RHI::RenderPassAttachment::LOAD_OP_DONTCARE, &rt};
    // renderer->GetDevice()->LoadRenderPass(cube_pass);

    // RHI::RenderPassHandle present_pass = {};
    // cube_pass.num_attachments = 1;
    // cube_pass.attachments[0] = {RHI::RenderPassAttachment::COLOR, RHI::RenderPassAttachment::LOAD_OP_CLEAR,
    // &backbuffer};

    // auto list = renderer->GetDevice()->BeginList();
    // renderer->GetDevice()->BeginFrame();
    // renderer->GetDevice()->EndFrame();

    editor->Initialize();

    auto imgui_ctx = editor->GetContext();
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(imgui_ctx);

    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // ImFontConfig config;
    // config.OversampleH = 7;
    // config.OversampleV = 7;

    // auto font = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Karla-Regular.ttf", 16, &config);

    // font->DisplayOffset.y = -1; // Render 1 pixel down

    SDL_Event event;
    bool running = true;
    int swap_chain_rebuild = false;

    // The main engine loop
    while (running) {
        PROFILING_SCOPE

        // Polling window events
        while (SDL_PollEvent(&event) != 0) {
            // ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED &&
                event.window.windowID == SDL_GetWindowID(window)) {
                swap_chain_rebuild = true;
            }

            if (event.type == SDL_QUIT)
                running = false;

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                running = false;
        }

        // Rebuild swapchain if needed (ex on resize)
        if (swap_chain_rebuild) {
            renderer->GetDevice()->RebuildSwapchain(renderer->GetSwapchain());
            swap_chain_rebuild = false;
        }

        // Resize viewport render target on viewporet size change
        if (renderer->resize) {
            renderer->ResizeRenderTargets();
            renderer->resize = false;
        }

        renderer->GetDevice()->BeginFrameEXP(renderer->GetSwapchain());

        rhi->Tick(0);
        renderer->Tick(0);
        editor->Tick(0);

        renderer->GetDevice()->EndFrameEXP(renderer->GetSwapchain());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    // ImGui_ImplSDL2_Shutdown();
    // ImGui_ImplRHI_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

} // namespace Squid