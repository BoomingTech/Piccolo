#pragma once

#include "runtime/core/log/log_system.h"

#include "runtime/function/global/global_context.h"

#include <chrono>
#include <thread>

#define LOG_SRC spdlog::source_loc(Pilot::LogSystem::FileName(__FILE__), __LINE__, __FUNCTION__)

#define LOG_HELPER(LOG_LEVEL, ...) g_runtime_global_context.m_logger_system->log(LOG_LEVEL, LOG_SRC, __VA_ARGS__);

#define LOG_DEBUG(...) LOG_HELPER(LogSystem::LogLevel::debug, __VA_ARGS__);

#define LOG_INFO(...) LOG_HELPER(LogSystem::LogLevel::info, __VA_ARGS__);

#define LOG_WARN(...) LOG_HELPER(LogSystem::LogLevel::warn, __VA_ARGS__);

#define LOG_ERROR(...) LOG_HELPER(LogSystem::LogLevel::error, __VA_ARGS__);

#define LOG_FATAL(...) LOG_HELPER(LogSystem::LogLevel::fatal, __VA_ARGS__);

#define PolitSleep(_ms) std::this_thread::sleep_for(std::chrono::milliseconds(_ms));

#define PolitNameOf(name) #name

#ifdef NDEBUG
#define ASSERT(statement)
#else
#define ASSERT(statement) assert(statement)
#endif
