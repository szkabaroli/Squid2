#include <Public/Core/Modules/ModuleManager.h>

namespace Squid {
namespace Core {

    void ModuleManager::RegisterStaticModule(const std::string module_name, ModuleDelegate delegate) {
        static_module_delegates.emplace(std::pair(module_name, delegate));
    }

} // namespace Core
} // namespace Squid
 