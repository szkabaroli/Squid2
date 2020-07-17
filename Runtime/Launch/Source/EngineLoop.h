#pragma once

namespace Squid {

class EngineLoop {
public:
    EngineLoop();
    ~EngineLoop();
    int Start();
    // void Pause();
    // void Terminate();
};

} // namespace Squid