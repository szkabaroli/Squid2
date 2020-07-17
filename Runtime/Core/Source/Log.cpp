#include <pch.h>
#include <Public/Core/Log.h>

namespace Squid {
namespace Core {

    void InitializeLogger(std::string name) {

        auto sink = std::make_shared<sink_mt>();
        auto logger = std::make_shared<spdlog::logger>(name, sink);
        spdlog::set_default_logger(logger);
    };

}; // namespace Core
} // namespace Squid
