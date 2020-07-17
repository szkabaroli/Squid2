#pragma once
#include <memory>
#include <map>
#include <string>

namespace Squid {
namespace Core {

    class IModule;

    // The EngineContext is a shared storage between different engine modules
    // so invidual modules can interact with each other
    class EngineContext {
    public:
        inline void SetModule(std::shared_ptr<IModule> module, std::string module_name) {
            modules.insert(std::pair(module_name, module));
        };

        template <typename T>
        inline std::shared_ptr<T> GetModule(std::string module_name) const {
            // We are casting dynamically once at the module registration so this would be unnecessary here.
            // <see cref="Fully.Qualified.Type.Name"/>
            return std::static_pointer_cast<T>(modules.at(module_name));
        }

    private:
        std::map<std::string, std::shared_ptr<IModule>> modules;
    };

} // namespace Core
} // namespace Squid
