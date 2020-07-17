#pragma once
#include <filesystem>
#include <chrono>
#include <map>
#include <string>
#include <functional>


namespace Squid { namespace  Core {

    enum class FileStatus {
        CREATED,
        UPDATED,
        DELETED
    };

    class FileWatcher {
    public:
        FileWatcher(const std::string &path, std::chrono::duration<int, std::milli> intervall);
        ~FileWatcher();
        void start(const std::function<void (std::string, FileStatus)> &action);

    private:
        std::string path;
        std::chrono::duration<int, std::milli> intervall;
        bool running = true;

        inline bool contains(const std::string &key) {
            auto el = paths.find(key);
            return el != paths.end();
        }

        std::unordered_map<std::string, std::filesystem::file_time_type> paths;
    };

}} // namespace Squid::Core
