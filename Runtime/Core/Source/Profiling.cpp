#include <Public/Core/Profiling.h>
#include <Public/Core/Log.h>

namespace Squid {
namespace Core {

    // == Timer class ==

    Timer::Timer() : running(false) {}

    void Timer::Start() {
        running = true;
        start_time = std::chrono::high_resolution_clock::now();
    }

    void Timer::Stop() {
        running = true;
        stop_time = std::chrono::high_resolution_clock::now();
    }

    f64 Timer::GetElapsedTime() {
        std::chrono::duration<f64, std::milli> fp_ms = stop_time - start_time;
        return fp_ms.count();
    }

    // == Profile class ==

    Profile::Profile(const std::string &name) : name(name), call_count(0), wall_time(0.0) {}

    Profile::~Profile() {}

    bool Profile::Start() {
        Profiler::GetInstance()->PushProfile(this);
        timer.Start();
        return true;
    }

    bool Profile::Stop() {
        Profiler::GetInstance()->PopProfile();
        timer.Stop(); // TODO: check if we need this line
        wall_time += timer.GetElapsedTime();
        ++call_count;
        return true;
    }

    // == Scoped Profile ==

    ScopedProfile::ScopedProfile(const std::string &name) : profile(nullptr) {
        std::string n(name);

        if (Profiler::GetInstance()->IsInStack(n)) { // profile is already in stack (probably a recursive call)
            if (true /*TODO: Profiler::GetInstance().GetOmitRecursiveCalls()*/) {
                return;
            } else {
                n = "RECURSIVE@" + n;
            }
        }

        profile = Profiler::GetInstance()->GetProfile(n);
        if (profile != nullptr) {
            if (!profile->Start()) { // cannot start profiler (probably a recursive call for flat profiler)
                delete profile;
                profile = NULL;
            }
        } else {
            LOG_ERROR("Cannot start scoped profiler: {}", n);
        }
    }

    ScopedProfile::~ScopedProfile() {
        if (profile != nullptr) {
            profile->Stop();
        }
    }

    // == Profiler class ==
    Profiler* Profiler::instance = nullptr;

    Profiler::Profiler() {}

    Profiler::~Profiler() {}

    std::map<std::string, Profile *> &Profiler::GetCurrentProfilesRoot() {
        return profile_stack.empty() ? profiles : profile_stack.back()->GetSubProfiles();
    }

    Profile *Profiler::GetProfile(const std::string &name) {
        std::map<std::string, Profile *> &profiles = GetCurrentProfilesRoot();

        std::map<std::string, Profile *>::iterator it = profiles.find(name);
        if (it != profiles.end()) {
            return it->second;
        } else {
            Profile *result = new Profile(name);
            profiles[name] = result;
            return result;
        }
    }

    void Profiler::CollectStats(std::ofstream &fs, std::map<std::string, Profile *> *p, int depth) {
        std::ostringstream oss;
        for (int i = 0; i < depth; ++i) {
            oss << "\t";
        }

        for (auto it = p->begin(); it != p->end(); ++it) {
            u32 cc = it->second->GetCallCount();
            f64 wall;
            it->second->GetTimes(wall);

            fs << oss.str() << it->second->GetName() << /*"  T(s):" << wall << */ "  #:" << cc
               << "  A(ms):" << (wall * 1000 / cc) * 0.001 << std::endl;
            CollectStats(fs, &(it->second->GetSubProfiles()), depth + 1);

            delete it->second;
        }
    }

    void Profiler::PrintStats() {
        std::ofstream fs;
        fs.open("profile.res");

        if (!fs.is_open()) {
            LOG_ERROR("Cannot open profiler output file: {}", "profile.res");
            return;
        }

        Profiler::CollectStats(fs, &(GetInstance()->profiles), 0);
        fs.close();

        //delete instance;
        //instance = nullptr;
    }

    void Profiler::PushProfile(Profile *p) { profile_stack.push_back(p); }

    void Profiler::PopProfile() {
        if (!profile_stack.empty()) {
            profile_stack.pop_back();
        }
    }

    bool Profiler::IsInStack(const std::string &name) {
        for (unsigned int i = 0; i < profile_stack.size(); ++i) {
            if (profile_stack[i]->GetName() == name) {
                return true;
            }
        }
        return false;
    }

} // namespace Core
} // namespace Squid