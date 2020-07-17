#pragma once
#include "Widget.h"
#include <pch.h>

namespace Squid {
namespace EditorCore {

    class LogWidget : public Widget {
    public:
        LogWidget();
        void Tick() override;

        void AddLogMessage(std::string msg);
        void Clear();

    private:
    };

} // namespace EditorCore
} // namespace Squid