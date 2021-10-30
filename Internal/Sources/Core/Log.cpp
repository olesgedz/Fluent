#include "Core/Log.hpp"

namespace Fluent::Log
{
    void SetLogLevel(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::eTrace:
            spdlog::set_level(spdlog::level::trace);
            break;
        case LogLevel::eInfo:
            spdlog::set_level(spdlog::level::info);
            break;
        case LogLevel::eWarn:
            spdlog::set_level(spdlog::level::warn);
            break;
        case LogLevel::eError:
            spdlog::set_level(spdlog::level::err);
            break;
        default:
            break;
        }
    }
} // namespace Fluent
