#pragma once

namespace Fluent
{
    enum class EventType
    {
        eKeyEvent, eKeyTypedEvent,
        eMouseButtonEvent,
        eMouseMoveEvent,
        eMouseScrollEvent,
        eWindowResizeEvent,
        eWindowCloseEvent,
        eMaxEvent
    };

    enum class PressState
    {
        eUndefined,
        ePress,
        eHold,
        eRelease
    };

    enum class ScrollDirection
    {
        eUp,
        eDown
    };

    class Event
    {
    public:
        virtual ~Event() noexcept = 0;

        virtual EventType GetType() const noexcept = 0;
    };

    class KeyEvent : public Event
    {
    public:
        KeyEvent(int key, PressState state)
            : mKey({ key, state }) {}
        ~KeyEvent() noexcept override = default;

        auto GetKey() const noexcept { return mKey; }
        
        EventType GetType() const noexcept override { return EventType::eKeyEvent; }
    private:
        struct Key
        {
            int         key;
            PressState  state;
        };

        Key mKey;
    };

    class KeyTypedEvent : public Event
    {
    public:
        explicit KeyTypedEvent(int codepoint) noexcept
            : mCodepoint(codepoint) {}
        ~KeyTypedEvent() noexcept override = default;

        int GetKey() const noexcept { return mCodepoint; }

        EventType GetType() const noexcept override { return EventType::eKeyTypedEvent; }
    private:
        int mCodepoint;
    };

    class MouseButtonEvent : public Event
    {
    public:
        MouseButtonEvent(int button, PressState state) noexcept
            : mButton({ button, state }) {}
        ~MouseButtonEvent() noexcept override = default;

        auto GetButton() const noexcept { return mButton; }

        EventType GetType() const noexcept override { return EventType::eMouseButtonEvent; }
    private:
        struct Button
        {
            int         button;
            PressState  state;
        };

        Button mButton;
    };

    class MouseMoveEvent : public Event
    {
    public:
        MouseMoveEvent(int x, int y) noexcept
            : mPosition({ x, y }) {}
        ~MouseMoveEvent() override = default;

        auto GetPosition() const noexcept { return mPosition; }

        EventType GetType() const noexcept override { return EventType::eMouseMoveEvent; }
    private:
        struct MousePosition
        {
            int x, y;
        };

        MousePosition mPosition;
    };

    class MouseScrollEvent : public Event
    {
    public:
        explicit MouseScrollEvent(ScrollDirection direction) noexcept
            : mDirection(direction) {};
        ~MouseScrollEvent() noexcept override = default;

        ScrollDirection GetDirection() const noexcept { return mDirection; }
        
        EventType GetType() const noexcept override { return EventType::eMouseScrollEvent; }
    private:
        ScrollDirection mDirection;
    };

    class WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(int width, int height) noexcept
            : mSize({ width, height }) {}
        ~WindowResizeEvent() noexcept override = default;

        auto GetSize() const noexcept { return mSize; }

        EventType GetType() const noexcept override { return EventType::eWindowResizeEvent; }
    private:
        struct WindowSize
        {
            int width, height;
        };

        WindowSize mSize;
    };

    class WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent() noexcept = default;
        ~WindowCloseEvent() noexcept override = default;
        
        EventType GetType() const noexcept override { return EventType::eWindowCloseEvent; }
    };

} // namespace Fluent