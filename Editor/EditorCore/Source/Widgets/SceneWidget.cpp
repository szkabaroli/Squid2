#include "SceneWidget.h"
#include <Core/Profiling.h>

namespace Squid {
namespace EditorCore {

    SceneWidget::SceneWidget(Renderer::Module *renderer) : renderer(renderer) { title = "SceneGraph"; }

    void SceneWidget::DrawChildrens(Core::Entity current, const std::string &name, u32 &index) {
        index++;
        auto &h = renderer->GetScene()->hierarchy;

        // Collect childrens
        std::vector<Core::Entity> childrens;
        for (int i = 0; i < h.GetCount(); i++) {
            if (h[i].parent_id == current) {
                childrens.push_back(h.GetEntity(i));
            }
        }

        bool has_children = childrens.size() != 0;
        bool tree_opened = false;

        ImVec4 color_odd = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        ImVec4 color_even = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

        // draw
        ImGui::PushStyleColor(ImGuiCol_Header, index % 2 != 0 ? color_odd : color_even);
        if (has_children) {
            if (ImGui::TreeNode(name.c_str())) {
                for (auto i = 0; i < childrens.size(); i++) {
                    auto child = childrens[i];
                    auto name = renderer->GetScene()->transforms.GetComponent(child)->name;
                    DrawChildrens(child, name, index);
                }

                ImGui::TreePop();
            }
        } else {
            ImGui::BulletText(name.c_str());
        }
        ImGui::PopStyleColor();

        // Loop through childrens and draw
    }

    void SceneWidget::Tick() {
        PROFILING_SCOPE

        if (Begin()) {

            char *data = "";
            ImGui::InputTextWithHint("s", "Search", data, 1000, 0);

            ImGui::BeginChild("name", ImVec2(0, 0), true);

            u32 index = 0;
            DrawChildrens(renderer->GetSceneRoot(), "Root", index);

            ImGui::EndChild();

            End();
        }
    }

} // namespace EditorCore
} // namespace Squid