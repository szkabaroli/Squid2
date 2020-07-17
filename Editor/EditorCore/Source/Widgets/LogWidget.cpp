#include "LogWidget.h"
#include <Core/Profiling.h>

namespace Squid {
namespace EditorCore {

    LogWidget::LogWidget() { title = "Log"; }

    void LogWidget::Tick() {
        PROFILING_SCOPE

        if (Begin()) {
            End();
        }
    }

} // namespace EditorCore
} // namespace Squid