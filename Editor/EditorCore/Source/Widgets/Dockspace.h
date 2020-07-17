#pragma once
#include "Widget.h"
#include <pch.h>

namespace Squid {
namespace EditorCore {

    class Dockspace {
    public:
        Dockspace();
        void Begin();
        void Tick();
        void End();

    private:
        bool editor_begun;
    };

} // namespace EditorCore
} // namespace Squid