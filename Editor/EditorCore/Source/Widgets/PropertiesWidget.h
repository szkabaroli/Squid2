#pragma once
#include "Widget.h"
#include <pch.h>

namespace Squid {
namespace EditorCore {

    class PropertiesWidget : public Widget {
    public:
        PropertiesWidget();
        void Tick() override;

    private:
    };

} // namespace EditorCore
} // namespace Squid