#pragma once

#include <string>
#include <cstdint>
#include <functional>
#include "Core/Base.hpp"

namespace Fluent
{
    class Event;
    using EventCallbackFn = std::function<void(const Event&)>;

    struct WindowDescription
    {
        uint32_t width;
        uint32_t height;
    };

    class Window
    {
    protected:
        Window() = default;
    public:
        virtual ~Window() = default;

        virtual bool ShouldClose() const = 0;
        virtual void OnUpdate() = 0;

        virtual void SetEventCallback(EventCallbackFn&& fn) = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        
        virtual Handle GetNativeHandle() const = 0;

        static Scope<Window> Create(const WindowDescription& description);
    };
}