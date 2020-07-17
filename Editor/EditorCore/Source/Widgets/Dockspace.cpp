

#include "Dockspace.h"
#include <Core/Profiling.h>

namespace Squid {
namespace EditorCore {

    Dockspace::Dockspace() {}

    void Dockspace::Begin() {
        PROFILING_SCOPE

        float offset_y = 0;
        // offset_y += _Editor::widget_menu_bar ? _Editor::widget_menu_bar->GetHeight() : 0;
        offset_y += 23; // Menu bar
                        // offset_y += 40; // Toolbar

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + offset_y));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - offset_y));

        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags window_flags = 0 | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
                                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        bool p_open = true;
        editor_begun = ImGui::Begin("Dockspace", &p_open, window_flags);

        if (editor_begun) {
            // Dock space
            const auto dock_main = ImGui::GetID("Dockspace");

            if (!ImGui::DockBuilderGetNode(dock_main)) {
                ImGui::DockBuilderRemoveNode(dock_main);
                ImGui::DockBuilderAddNode(dock_main, ImGuiDockNodeFlags_None);
                ImGui::DockBuilderSetNodeSize(dock_main, ImGui::GetMainViewport()->Size);

                // Layout
                ImGuiID dock_main_id = dock_main;

                ImGuiID dock_right_id =
                    ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);

                ImGuiID dock_left_id =
                    ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);

                ImGuiID dock_down_id =
                    ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);

                // Build
                ImGui::DockBuilderDockWindow("Properties", dock_right_id);
                ImGui::DockBuilderDockWindow("Dear ImGui Demo", dock_right_id);
                ImGui::DockBuilderDockWindow("SceneGraph", dock_left_id);
                ImGui::DockBuilderDockWindow("Log", dock_down_id);
                ImGui::DockBuilderDockWindow("Viewport", dock_main_id);

                ImGui::DockBuilderFinish(dock_main_id);
            }

            ImGui::DockSpace(dock_main, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::PopStyleVar(4);
        }
    }

    void Dockspace::Tick() { PROFILING_SCOPE }

    void Dockspace::End() {
        PROFILING_SCOPE

        if (editor_begun)
            ImGui::End();
    }

} // namespace EditorCore
} // namespace Squid