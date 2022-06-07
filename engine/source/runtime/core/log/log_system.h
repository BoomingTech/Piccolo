#pragma once

#include <cstdint>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace Pilot
{

    class LogSystem final
    {
    public:
        enum class LogLevel : uint8_t
        {
            debug,
            info,
            warn,
            error,
            fatal
        };

        static constexpr const char* FileName(const char* path)
        {
            const char* file = path;
            while (*path)
            {
                if (*path++ == std::filesystem::path::preferred_separator)
                {
                    file = path;
                }
            }
            return file;
        }

    public:
        LogSystem(const std::string& log_pattern);
        ~LogSystem();

        template<typename... TARGS>
        void log(LogLevel level, spdlog::source_loc&& loc, TARGS&&... args)
        {
            switch (level)
            {
                case LogLevel::debug:
                    m_logger->log(
                        std::forward<spdlog::source_loc>(loc), spdlog::level::debug, std::forward<TARGS>(args)...);
                    break;
                case LogLevel::info:
                    m_logger->log(
                        std::forward<spdlog::source_loc>(loc), spdlog::level::info, std::forward<TARGS>(args)...);
                    break;
                case LogLevel::warn:
                    m_logger->log(
                        std::forward<spdlog::source_loc>(loc), spdlog::level::warn, std::forward<TARGS>(args)...);
                    break;
                case LogLevel::error:
                    m_logger->log(
                        std::forward<spdlog::source_loc>(loc), spdlog::level::err, std::forward<TARGS>(args)...);
                    break;
                case LogLevel::fatal:
                    m_logger->log(
                        std::forward<spdlog::source_loc>(loc), spdlog::level::critical, std::forward<TARGS>(args)...);
                    fatalCallback(std::forward<TARGS>(args)...);
                    break;
                default:
                    break;
            }
        }

        template<typename... TARGS>
        void fatalCallback(TARGS&&... args)
        {
            const std::string format_str = fmt::format(std::forward<TARGS>(args)...);
            throw std::runtime_error(format_str);
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger;
    };

} // namespace Pilot