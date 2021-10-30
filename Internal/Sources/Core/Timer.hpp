#pragma once

#include <chrono>

namespace Fluent
{
    class Timer
    {
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> mStart;
    public:
        Timer() noexcept
        {
            Reset();
        }

        void Reset() noexcept
        {
            mStart = std::chrono::high_resolution_clock::now();
        }

        float Elapsed() noexcept
        {
            using namespace std::chrono;
            auto elapsed = duration_cast<nanoseconds>(high_resolution_clock::now() - mStart).count();
            return static_cast<float>(elapsed) * 0.001f * 0.001f * 0.001f;
        }
    };
}