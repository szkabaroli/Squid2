#include <Public/Core/FileWatcher.h>
#include <thread>

namespace Squid { namespace Core {

    FileWatcher::FileWatcher(
        const std::string &path, std::chrono::duration<int, std::milli> intervall)
        : intervall(intervall), path(path) {
            // Get the initial files to watch
            for(auto& file : std::filesystem::recursive_directory_iterator(path)) {
                paths[file.path().string()] = std::filesystem::last_write_time(file);
            }
        }

    FileWatcher::~FileWatcher() {}

    void FileWatcher::start(const std::function<void(std::string, FileStatus)> &action) {
        while (running) {
            std::this_thread::sleep_for(intervall);

            auto it = paths.begin();

            while(it != paths.end()) {
                if(!std::filesystem::exists(it->first)) {
                    action(it->first, FileStatus::DELETED);
                    it = paths.erase(it);
                } else {
                    it++;
                }
            }

            for(auto& file : std::filesystem::recursive_directory_iterator(path)) {
                auto last_write = std::filesystem::last_write_time(file);
                auto path_string = file.path().string();

                if(!contains(path_string)) {
                    paths[path_string] = last_write;
                    action(path_string, FileStatus::CREATED);
                } else {
                    if(paths[path_string] != last_write) {
                        action(path_string, FileStatus::UPDATED);
                        paths[path_string] = last_write;
                    }
                }
            }
        }
    }

}} // namespace Squid::Core
