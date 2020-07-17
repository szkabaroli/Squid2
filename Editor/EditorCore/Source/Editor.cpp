#include <pch.h>
#include "Editor.h"
#include <Core/Profiling.h>

#include "Widgets/Dockspace.h"
#include "Widgets/LogWidget.h"
#include "Widgets/SceneWidget.h"
#include "Widgets/PropertiesWidget.h"
#include "Widgets/ViewportWidget.h"

#include <Core/FileDialog.h>
#include <sstream>

namespace Squid {
namespace EditorCore {

    Editor::Editor(std::shared_ptr<Renderer::Module> renderer) : renderer(renderer) {
        // Setup style
        this->SetupStyles();

        // Create widgets
        // INFO: We can use raw pointers here becouse the lifetime of the widgets should never outlive the renderer
        this->dockspace = std::make_unique<Dockspace>();
        this->scene_widget = std::make_unique<SceneWidget>(renderer.get());
        this->viewport_widget = std::make_unique<ViewportWidget>(renderer.get());
        this->properties_widget = std::make_unique<PropertiesWidget>();
        this->log_widget = std::make_unique<LogWidget>();
    }

    Editor::~Editor() {}

    static std::unique_ptr<pfd::open_file> open_file;

    void Editor::Tick() {
        PROFILING_SCOPE

        /* ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + 23));
         ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 40));
         ImGui::SetNextWindowViewport(viewport->ID);

         ImGuiWindowFlags window_flags2 = 0 | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                          ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
         ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
         ImGui::Begin("TOOLBAR", NULL, window_flags2);
         ImGui::PopStyleVar();
         ImGui::SameLine();
         ImGui::Button("Play", ImVec2(0, 26));
         ImGui::SameLine();
         ImGui::Button("2", ImVec2(0, 26));
         ImGui::SameLine();
         ImGui::Button("3", ImVec2(0, 26));

         ImGui::End();*/

        EditorBegin();
        std::vector<std::string> filters = {"Image Files", "*.png *.jpg *.jpeg *.bmp"};

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        if (ImGui::BeginMainMenuBar()) {

            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open Project", "Ctrl+O"))
                    open_file = std::make_unique<pfd::open_file>("Choose file", "C:\\", filters);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Windows")) {

                if (ImGui::MenuItem("Scene Hierarchy", nullptr, &scene_widget->GetVisible()))
                    scene_widget->SetVisible(!scene_widget->GetVisible());

                if (ImGui::MenuItem("Log", nullptr, &log_widget->GetVisible()))
                    log_widget->SetVisible(!log_widget->GetVisible());

                if (ImGui::MenuItem("Properties", nullptr, &properties_widget->GetVisible()))
                    properties_widget->SetVisible(!properties_widget->GetVisible());

                if (ImGui::MenuItem("Viewport", nullptr, &viewport_widget->GetVisible()))
                    viewport_widget->SetVisible(!viewport_widget->GetVisible());

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar();

        // ImGui::ShowDemoWindow(&p_open_test);

        // TODO widgets
        viewport_widget->Tick();
        scene_widget->Tick();
        properties_widget->Tick();
        log_widget->Tick();

        auto t = true;
        std::stringstream ss;
        std::stringstream ss2;
        ImGui::Begin("Test", &t, 0);
        ss << renderer->GetFrameHeight() << " " << renderer->GetFrameWidth() << std::endl;
        ss2 << renderer->GetFrame().height << " " << renderer->GetFrame().width << std::endl;

        ImGui::Text(ss.str().c_str());
        ImGui::Text(ss2.str().c_str());
        ImGui::End();
        // ImGui::PushItemFlag(ImGuiItemFlags_Disabled, (bool)open_file);

        if (open_file && open_file->ready()) {
            auto result = open_file->result();
            if (result.size() > 0)
                std::cout << "Opened file " << result[0] << "\n";
            open_file.reset();
        }

        // ImGui::PopItemFlag();

        EditorEnd();
    }

    void Editor::EditorBegin() {
        dockspace->Begin();
        dockspace->Tick();
    }

    void Editor::EditorEnd() { dockspace->End(); }

    void Editor::SetupStyles() const {
        // Colors
        ImGui::StyleColorsDark();

        auto blue = ImVec4(0.0f / 255.0f, 122.0f / 255.0f, 204.0f / 255.0f, 1.00f);

        auto black = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
        auto gray08 = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        auto gray11 = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);
        auto gray12 = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
        auto gray13 = ImVec4(0.13f, 0.13f, 0.14f, 1.00f);
        auto gray14 = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
        auto gray20 = ImVec4(0.2f, 0.2f, 0.21f, 1.00f);
        auto gray30 = ImVec4(0.3f, 0.3f, 0.31f, 1.00f);

        ImVec4 *colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = gray12;
        colors[ImGuiCol_Border] = ImVec4(0.40f, 0.40f, 0.40f, 0.50f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        colors[ImGuiCol_FrameBg] = gray20;
        colors[ImGuiCol_FrameBgHovered] = gray30;
        colors[ImGuiCol_FrameBgActive] = gray30;

        colors[ImGuiCol_TitleBg] = gray08;
        colors[ImGuiCol_TitleBgActive] = gray08;
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);

        colors[ImGuiCol_Header] = gray20;
        colors[ImGuiCol_HeaderHovered] = gray30;
        colors[ImGuiCol_HeaderActive] = gray30;

        colors[ImGuiCol_Button] = gray20;
        colors[ImGuiCol_ButtonHovered] = gray30;
        colors[ImGuiCol_ButtonActive] = gray30;

        colors[ImGuiCol_MenuBarBg] = gray08;
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark] = blue;
        colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

        colors[ImGuiCol_Separator] = gray08;
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);

        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

        colors[ImGuiCol_Tab] = gray14;
        colors[ImGuiCol_TabHovered] = blue;
        colors[ImGuiCol_TabActive] = blue;
        colors[ImGuiCol_TabUnfocused] = gray14;
        colors[ImGuiCol_TabUnfocusedActive] = gray20;

        colors[ImGuiCol_DockingPreview] = blue;
        colors[ImGuiCol_DockingEmptyBg] = gray08;

        colors[ImGuiCol_WindowBg].w = 1.0f;

        // Spatial settings
        const auto font_size = 24.0f;
        const auto font_scale = 0.7f;
        const auto roundness = 2.0f;

        ImGuiStyle &style = ImGui::GetStyle();

        style.WindowBorderSize = 1.0f;
        style.ScrollbarSize = 15.0f;
        style.FramePadding = ImVec2(5, 5);
        style.ItemSpacing = ImVec2(6, 5);
        style.WindowMenuButtonPosition = ImGuiDir_Right;
        style.WindowRounding = roundness;
        style.FrameRounding = roundness;
        style.FrameBorderSize = 1.0f;
        style.PopupRounding = roundness;
        style.GrabRounding = roundness;
        style.ScrollbarRounding = roundness;
        style.TabRounding = roundness;
        style.Alpha = 1.0f;
    }

} // namespace EditorCore
} // namespace Squid