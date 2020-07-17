#pragma once
#include "Types.h"
#include <map>
#include <sstream>
#include <string>
#include <fstream>
#include <chrono>
#include <vector>

#define PROFILING_SCOPE                                                                                                \
    std::ostringstream _profiling_oss;                                                                                 \
    _profiling_oss << /*__FILE__ << ":" << */ __FUNCTION__ << ":" << __LINE__;                                         \
    Squid::Core::ScopedProfile _sco_pro(_profiling_oss.str());

#define PROFILING_NAMED_SCOPE(NAME)                                                                                    \
    std::ostringstream _profiling_oss;                                                                                 \
    _profiling_oss << (NAME);                                                                                          \
    Squid::Core::ScopedProfile _sco_pro(_profiling_oss.str());

namespace Squid {
namespace Core {

    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

    class Timer {
    public:
        Timer();
        void Start();
        void Stop();
        f64 GetElapsedTime();

    private:
        TimePoint start_time;
        TimePoint stop_time;
        bool running;
    };

    class Profile {
        friend class Profiler;
        friend class ScopedProfile;

    private:
        Profile(const std::string &name);
        ~Profile();
        bool Start();
        bool Stop();

        inline u32 GetCallCount() const { return call_count; }
        inline std::string GetName() const { return name; }
        inline void GetTimes(f64 &wall) const { wall = wall_time; };
        inline std::map<std::string, Profile *> &GetSubProfiles() { return sub_profiles; };

        std::map<std::string, Profile *> sub_profiles;

        std::string name;
        u32 call_count;
        f64 wall_time;
        Timer timer;
    };

    class ScopedProfile {
    public:
        ScopedProfile(const std::string &name);
        ~ScopedProfile();

    private:
        Profile *profile;
    };

    class Profiler {
        friend class Profile;
        friend class ScopedProfile;

    public:
        static void PrintStats();

    private:
        Profiler();
        ~Profiler();

        static Profiler *GetInstance() {
            if (instance == nullptr) {
                instance = new Profiler();
                atexit(PrintStats);
            }
            return instance;
        };

        static Profiler *instance;

        Profile *GetProfile(const std::string &name);

        void PushProfile(Profile *p);
        void PopProfile();
        bool IsInStack(const std::string &name);
        std::map<std::string, Profile *> &Profiler::GetCurrentProfilesRoot();

        static void CollectStats(std::ofstream &fs, std::map<std::string, Profile *> *p, int depth);

        // TODO: Maybe unordered?
        std::map<std::string, Profile *> profiles;
        std::vector<Profile *> profile_stack;
    };

} // namespace Core
} // namespace Squid