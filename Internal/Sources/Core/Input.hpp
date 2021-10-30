#pragma once

#include <array>
#include "Core/Base.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/MouseCodes.hpp"

namespace Fluent
{
    class Input
    {
    public:
        static void Init() noexcept;
        static void OnUpdate() noexcept;

        static void OnEvent(const Event& event) noexcept;
        
        static bool GetKeyDown(KeyCode keycode) noexcept;
        static bool GetKey(KeyCode keycode) noexcept;
        static bool GetKeyUp(KeyCode keycode) noexcept;

        static bool GetButton(MouseCode button) noexcept;
        static bool GetButtonUp(MouseCode button) noexcept;

        static int GetMouseX() noexcept;
        static int GetMouseY() noexcept;
    };
} // namespace Fluent