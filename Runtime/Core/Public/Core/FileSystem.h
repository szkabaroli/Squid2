#pragma once
#include <fstream>
#include <string>

namespace Squid {
namespace Core {

    static std::string ReadTextFile(const char *file_name) {
        std::ifstream file(file_name, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        std::string buffer(size, ' ');
        file.seekg(0);
        file.read(&buffer[0], size);
        file.close();
        return buffer;
    }

} // namespace Core
} // namespace Squid