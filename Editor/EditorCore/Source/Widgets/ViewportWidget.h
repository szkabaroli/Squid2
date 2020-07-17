#pragma once
#include <pch.h>
#include "Widget.h"
#include <Renderer/Module.h>

namespace Squid {
namespace EditorCore {

    class ViewportWidget : public Widget {
    public:
        ViewportWidget(Renderer::Module *renderer);
        void Tick() override;

    private:
        Renderer::Module *renderer;
        ImVec2 v_size;
    };

} // namespace EditorCore
} // namespace Squid