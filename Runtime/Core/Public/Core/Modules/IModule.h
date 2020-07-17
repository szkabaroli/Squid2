#pragma once
#include "EngineContext.h"

#ifdef SQUID_WIN32 // && !SQUID_STATIC_LINK_MODULES
#define SQUID_API __declspec(dllexport)
#else
#define SQUID_API
#endif // SQUID_WIN32

namespace Squid {
namespace Core {

    template <class M>
    class StaticModule;

#ifdef SQUID_STATIC_LINK_MODULES
#define IMPLEMENT_MODULE(ModuleClass, module_name)                                                                     \
    static StaticModule<ModuleClass> static_module_##module_name(std::string(#module_name));
#else
#define IMPLEMENT_MODULE(ModuleClass, module_name)                                                                     \
    extern "C" SQUID_API ModuleClass *InitializeModule(Squid::Core::EngineContext *context) {                          \
        Squid::Core::InitializeLogger(#module_name);                                                                   \
        return new ModuleClass(context);                                                                               \
    }                                                                                                                  \
    extern "C" SQUID_API void ReleaseModule(ModuleClass *ptr) { delete ptr; }
#endif

    class SQUID_API IModule {
    public:
        IModule(EngineContext *context) { this->context = context; }
        virtual ~IModule() = default;
        virtual bool Initialize() { return true; };
        virtual void Event(){};
        virtual void Tick(float delta_time){};

    protected:
        EngineContext *context;
    };

} // namespace Core
} // namespace Squid