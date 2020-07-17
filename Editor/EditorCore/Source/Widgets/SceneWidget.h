#pragma once
#include "Widget.h"
#include <pch.h>

#include <Renderer/Module.h>

namespace Squid {
namespace EditorCore {

    class SceneWidget : public Widget {
    public:
        SceneWidget(Renderer::Module* renderer);
        void Tick() override;
        void DrawChildrens(Core::Entity current, const std::string &name, u32 &index);

    private:
        Renderer::Module* renderer;
    };

} // namespace EditorCore
} // namespace Squid