#include <cassert>
#include <chrono>
#include <vector>
#include <pch.h>

#include <Core/Log.h>
#include <RHI/Module.h>

#include "EngineLoop.h"

using namespace Squid;
using namespace Core;

int main() {
    
    Core::InitializeLogger("main");

    auto app = new EngineLoop();
    app->Start();
    delete app;
    return 0;
}
