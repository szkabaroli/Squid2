#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <stack>

#include "IModule.h"
#include "EngineContext.h"

#ifndef SQUID_STATIC_LINK_MODULES
#ifdef SQUID_WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif // SQUID_WIN32
#endif // SQUID_STATIC_LINK_MODULES

namespace Squid {
namespace Core {

    typedef IModule *(*ModuleDelegateRaw)(EngineContext *ctx);
    typedef std::function<IModule *(EngineContext *ctx)> ModuleDelegate;

    typedef void (*ModuleDelegateReleaseRaw)(IModule *);
    typedef std::function<void(IModule *)> ModuleDelegateRelease;

    class ModuleManager {
    public:
        static ModuleManager &GetInstance() {
            static ModuleManager instance;
            return instance;
        }
        // Register static or dynamic module
        template <typename T>
        std::optional<std::shared_ptr<T>> RegisterModule(EngineContext *ctx, std::string module_name) {

#ifdef SQUID_STATIC_LINK_MODULES
            ModuleDelegate initialize_module = nullptr;

            for (auto module_delegate : static_module_delegates) {
                if (module_delegate.first == module_name) {
                    initialize_module = module_delegate.second;
                }
            }
#else
#ifdef SQUID_WIN32
            std::string lib_name = module_name;
            lib_name.append(".dll");
            HINSTANCE lib = LoadLibrary(lib_name.c_str());

            if (!lib)
                return std::nullopt;

            ModuleDelegateRaw InitializeModule = (ModuleDelegateRaw)GetProcAddress(lib, "InitializeModule");
            ModuleDelegateReleaseRaw ReleaseModule = (ModuleDelegateReleaseRaw)GetProcAddress(lib, "ReleaseModule");
#else
            std::string lib_name = "lib";
            lib_name.append(module_name);
            lib_name.append(".so");

            void *lib = dlopen(lib_name.c_str(), RTLD_LAZY);

            if (!lib)
                return std::nullopt;

            ModuleDelegateRaw InitializeModule = (ModuleDelegateRaw)dlsym(lib, "InitializeModule");
            ModuleDelegateReleaseRaw ReleaseModule = (ModuleDelegateReleaseRaw)dlsym(lib, "ReleaseModule");
#endif // SQUID_WIN32
#endif // SQUID_STATIC_LINK_MODULES

            if (!InitializeModule)
                return std::nullopt;

            auto module = InitializeModule(ctx);
            auto casted = dynamic_cast<T *>(module);

            if (!casted)
                return std::nullopt;

            std::shared_ptr<T> ptr = std::shared_ptr<T>(casted, [=](auto ptr) {
                ReleaseModule(ptr);
                // TODO: Free library
            });

            return ptr;
            // releases.insert(std::pair(module, ReleaseModule));
            // context->AddModule(module, std::move(module_name.c_str()));
        };

        void RegisterStaticModule(const std::string module_name, ModuleDelegate delegate);

    private:
        std::map<std::string, ModuleDelegate> static_module_delegates;
        std::map<IModule *, ModuleDelegateReleaseRaw> releases;

        ModuleManager() {}
    };

    template <class M>
    class StaticModule {
    public:
        StaticModule(std::string module_name) {
            // create delegate
            ModuleDelegate delegate = std::bind(&StaticModule<M>::InitializeModule, this);
            // Register this module
            ModuleManager::GetInstance().RegisterStaticModule(module_name, delegate);
        }

        IModule *InitializeModule(EngineContext *ctx) { return new M(ctx); }
    };

} // namespace Core
} // namespace Squid
