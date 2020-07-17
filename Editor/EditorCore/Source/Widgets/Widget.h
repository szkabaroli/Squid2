#pragma once
#include <Core/Types.h>
#include <pch.h>

namespace Squid {
namespace EditorCore {

    class Widget {
    public:
        virtual ~Widget(){};

        bool Begin() {
            assert(title.length() > 0);

            if (!is_visible)
                return false;

            // Position
            if (position.x != -1.0f && position.y != -1.0f) {
                ImGui::SetNextWindowPos(position);
            }

            // Padding
            if (padding.x != -1.0f && padding.y != -1.0f) {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
                var_pushes++;
            }

            // Size
            if (size.x != -1.0f && size.y != -1.0f) {
                ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
            }

            // Max size
            if ((size.x != -1.0f && size.y != -1.0f) || (size_max.x != FLT_MAX && size_max.y != FLT_MAX)) {
                ImGui::SetNextWindowSizeConstraints(size, size_max);
            }

            // Frame Border
            if (frame_border != -1.0f) {
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, frame_border);
                var_pushes++;
            }

            // Window Border
            if (window_border != -1.0f) {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, window_border);
                var_pushes++;
            }

            // Window Rounding
            if (window_rounding != -1.0f) {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, window_rounding);
                var_pushes++;
            }

            // Begin

            if (ImGui::Begin(title.c_str(), &is_visible, flags)) {
                window = ImGui::GetCurrentWindow();
                begun = true;
            } else if (window && window->Hidden) {
                // Enters here if the window is hidden as part of an unselected tab.
                // ImGui::Begin() makes the window and but returns false, then ImGui still expects ImGui::End() to be
                // called. So we make sure that when Widget::End() is called, ImGui::End() get's called as well. Note:
                // ImGui's docking is in beta, so maybe it's at fault here ?
                begun = true;
            }

            viewport = ImGui::GetWindowViewport();
            return begun;
        }

        void End() {
            // End

            if (begun) {
                ImGui::End();
            }

            // Pop style variables
            ImGui::PopStyleVar(var_pushes);
            var_pushes = 0;

            begun = false;
        }

        inline bool &GetVisible() { return is_visible; }
        inline void SetVisible(bool is_visible) { is_visible = is_visible; }

        virtual void Tick() = 0;

    protected:
        static constexpr float MAX_FLOAT = std::numeric_limits<float>::max();

        bool is_visible = true;

        i32 flags = ImGuiWindowFlags_NoCollapse;

        ImVec2 position = {-1.0f, -1.0f};
        ImVec2 size = {-1.0f, -1.0f};
        ImVec2 size_max = {MAX_FLOAT, MAX_FLOAT};
        ImVec2 padding = {-1.0f, -1.0f};

        f32 frame_border = -1.0f;
        f32 window_border = -1.0f;
        f32 window_rounding = -1.0f;

        ImGuiViewport *viewport = nullptr;
        std::string title;

        ImGuiWindow *window = nullptr;
        bool begun = false;

    private:
        u8 var_pushes = 0;
    };

} // namespace EditorCore
} // namespace Squid