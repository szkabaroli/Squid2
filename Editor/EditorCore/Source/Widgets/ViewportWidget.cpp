#include "ViewportWidget.h"
#include <Core/Profiling.h>

namespace Squid {
namespace EditorCore {

    ViewportWidget::ViewportWidget(Renderer::Module *renderer) : renderer(renderer) {
        title = "Viewport";
        padding = ImVec2(5.0f, 5.0f);
    }

    void ViewportWidget::Tick() {
        PROFILING_SCOPE

        if (Begin()) {

            // ImVec2 v_min = ImGui::GetWindowContentRegionMin();
            // ImVec2 v_max = ImGui::GetWindowContentRegionMax();
            // v_min.x += ImGui::GetWindowPos().x;
            // v_min.y += ImGui::GetWindowPos().y;
            // v_max.x += ImGui::GetWindowPos().x;
            // v_max.y += ImGui::GetWindowPos().y;

            // Get current frame window resolution
            uint32_t width =
                static_cast<uint32_t>(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x);
            uint32_t height =
                static_cast<uint32_t>(ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y);
            // const uint32_t max_res = m_renderer->GetMaxResolution();
            // if (width > max_res || height > max_res)
            //    return;

            // Make pixel perfect
            width -= (width % 2 != 0) ? 1 : 0;
            height -= (height % 2 != 0) ? 1 : 0;

            const auto &current_height = renderer->GetFrameHeight();
            const auto &current_width = renderer->GetFrameWidth();

            if (current_width != width || current_height != height) {
                renderer->SetFrameSize(width, height);
            }

            ImGui::Image(
                (uint64_t *)&renderer->GetFrame(), ImVec2(static_cast<f32>(width), static_cast<f32>(height)),
                ImVec2(0, 0), ImVec2(1, 1));

            End();
        }
    }

} // namespace EditorCore
} // namespace Squid