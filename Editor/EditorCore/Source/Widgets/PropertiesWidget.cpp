#include "PropertiesWidget.h"
#include <Core/Profiling.h>

namespace Squid {
namespace EditorCore {

PropertiesWidget::PropertiesWidget() { title = "Properties"; }

    void PropertiesWidget::Tick() {
        PROFILING_SCOPE

        if (Begin()) {
            End();
        }
    }

}
} // namespace Squid