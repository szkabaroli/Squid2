#pragma once
#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/null_mutex.h"
#include <mutex>

namespace Squid {
namespace Core {

    template <typename Mutex>
    class CustomSink : public spdlog::sinks::base_sink<Mutex> {
    protected:
        void sink_it_(const spdlog::details::log_msg &msg) override {
            spdlog::memory_buf_t formatted;
            base_sink<Mutex>::formatter_->format(msg, formatted);
            std::cout << "custom > " << fmt::to_string(formatted);
        }

        void flush_() override { std::cout << std::flush; }
    };

    using sink_mt = CustomSink<std::mutex>;
    using sink_st = CustomSink<spdlog::details::null_mutex>;

    void InitializeLogger(std::string name);
}; // namespace Core
} // namespace Squid

#define LOG(text, ...) spdlog::info(text, __VA_ARGS__);
#define LOG_DEBUG(text, ...) spdlog::debug(text, __VA_ARGS__);
#define LOG_INFO(text, ...) spdlog::info(text, __VA_ARGS__);
#define LOG_WARN(text, ...) spdlog::warn(text, __VA_ARGS__);
#define LOG_ERROR(text, ...) spdlog::error(text, __VA_ARGS__);
#define LOG_CRITICAL(text, ...) spdlog::critical(text, __VA_ARGS__);