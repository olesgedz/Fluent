#include <array>
#include <set>
#include "Core/Window.hpp"
#include "Core/Event.hpp"
#include "Math/Math.hpp"
#include "Core/Input.hpp"

namespace Fluent
{
    using MousePosition = VectorInt2;

    static std::array<PressState, 349> keys;
    static std::array<PressState, 5> buttons;
    static std::set<std::pair<KeyCode, PressState>> keysChanged;
    static MousePosition mousePosition(0, 0);

    void Input::Init() noexcept
    {
        keys.fill(PressState::eUndefined);
        buttons.fill(PressState::eUndefined);
    }

    void Input::OnUpdate() noexcept
    {
        for (auto& [keyCode, state] : keysChanged)
        {
            if (state == PressState::eRelease && keys[keyCode] == PressState::eRelease)
                keys[keyCode] = PressState::eUndefined;
            else
            if (state == PressState::ePress && keys[keyCode] == PressState::ePress)
                keys[keyCode] = PressState::eHold;
            else
                keys[keyCode] = state;
        }

        keysChanged.clear();
    }

    void Input::OnEvent(const Event& event) noexcept
    {
        switch (event.GetType())
        {
            case EventType::eKeyEvent:
            {
                auto key = dynamic_cast<const KeyEvent&>(event).GetKey();
                if (key.key >= 0 && key.key < 349) // TODO: Rewrite
                {
                    keys[key.key] = static_cast<PressState>(key.state);
                    keysChanged.insert(std::make_pair(key.key, key.state));
                }
                break;
            }
            case EventType::eMouseButtonEvent:
            {
                auto button = dynamic_cast<const MouseButtonEvent&>(event).GetButton();
                buttons[button.button] = static_cast<PressState>(button.state);
                break;
            }
            case EventType::eMouseMoveEvent:
            {
                auto position = dynamic_cast<const MouseMoveEvent&>(event).GetPosition();
                mousePosition = { position.x, position.y };
                break;
            }
            default:
                break;
        }
    }

    bool Input::GetKeyDown(KeyCode keycode) noexcept
    {
        return keys[keycode] == PressState::ePress;
    }

    bool Input::GetKey(KeyCode keycode) noexcept
    {
        return keys[keycode] == PressState::eHold || keys[keycode] == PressState::ePress;
    }

    bool Input::GetKeyUp(KeyCode keycode) noexcept
    {
        return keys[keycode] == PressState::eRelease;
    }

    bool Input::GetButton(MouseCode button) noexcept
    {
        return buttons[button] == PressState::ePress;
    }

    bool Input::GetButtonUp(MouseCode button) noexcept
    {
        return buttons[button] == PressState::eRelease;
    }

    int Input::GetMouseX() noexcept
    {
        return mousePosition.x;
    }

    int Input::GetMouseY() noexcept
    {
        return mousePosition.y;
    }
} // namespace Fluent