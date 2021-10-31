#include <GLFW/glfw3.h>
#include <iostream>
#include "Core/Base.hpp"
#include "Core/Window.hpp"
#include "Core/Event.hpp"

namespace Fluent
{
    class MultiplatformWindow : public Window
    {
    private:
        GLFWwindow*         mHandle;
        uint32_t            mWidth;
        uint32_t            mHeight;
        std::string         mTitle;
        EventCallbackFn     mEventCallback;

        void SendEvent(const Event& event)
        {
            if (event.GetType() == EventType::eWindowResizeEvent)
            {
                auto size = dynamic_cast<const WindowResizeEvent&>(event).GetSize();
                mWidth = size.width;
                mHeight = size.height;
            }

            mEventCallback(event);
        }
    public:
        MultiplatformWindow(const WindowDescription& description)
            : mWidth(description.width), mHeight(description.height) 
        {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            mHandle = glfwCreateWindow(mWidth, mHeight, mTitle.c_str(), nullptr, nullptr);
            glfwSetWindowUserPointer(mHandle, this);

            glfwSetKeyCallback(mHandle, [](GLFWwindow* window, int key, int scancode, int action, int mods)
            {
                auto* handle = reinterpret_cast<MultiplatformWindow*>(glfwGetWindowUserPointer(window));
                if (action == GLFW_PRESS)
                    handle->SendEvent(KeyEvent(key, PressState::ePress));
                else
                if (action == GLFW_RELEASE)
                    handle->SendEvent(KeyEvent(key, PressState::eRelease));
            });

            glfwSetCharCallback(mHandle, [](GLFWwindow* window, uint32_t codepoint)
            {
                auto* handle = reinterpret_cast<MultiplatformWindow*>(glfwGetWindowUserPointer(window));
                handle->SendEvent(KeyTypedEvent(codepoint));
            });

            glfwSetMouseButtonCallback(mHandle, [](GLFWwindow* win, int button, int action, int mods)
            {
                auto* handle = reinterpret_cast<MultiplatformWindow*>(glfwGetWindowUserPointer(win));
                if (action == GLFW_PRESS)
                    handle->SendEvent(MouseButtonEvent(button, PressState::ePress));
                else
                if (action == GLFW_RELEASE)
                    handle->SendEvent(MouseButtonEvent(button, PressState::eRelease));
            });

            glfwSetScrollCallback(mHandle, [](GLFWwindow* window, double xOffset, double yOffset)
            {
                auto* handle = reinterpret_cast<MultiplatformWindow*>(glfwGetWindowUserPointer(window));

                if (yOffset > 0)
                    handle->SendEvent(MouseScrollEvent(ScrollDirection::eUp));
                else
                if (yOffset < 0)
                    handle->SendEvent(MouseScrollEvent(ScrollDirection::eDown));
            });

            glfwSetWindowCloseCallback(mHandle, [](GLFWwindow* window)
            {
                auto* handle = reinterpret_cast<MultiplatformWindow*>(glfwGetWindowUserPointer(window));
                handle->SendEvent(WindowCloseEvent());
            });

            glfwSetWindowSizeCallback(mHandle, [](GLFWwindow* window, int width, int height)
            {
                auto* handle = reinterpret_cast<MultiplatformWindow*>(glfwGetWindowUserPointer(window));
                handle->SendEvent(WindowResizeEvent(width, height));
            });

            glfwSetFramebufferSizeCallback(mHandle, [](GLFWwindow* window, int width, int height)
            {
                auto* handle = reinterpret_cast<MultiplatformWindow*>(glfwGetWindowUserPointer(window));
                handle->SendEvent(WindowResizeEvent(width, height));
            });

            glfwSetCursorPosCallback(mHandle, [](GLFWwindow* window, double x, double y)
            {
                auto* handle = reinterpret_cast<MultiplatformWindow*>(glfwGetWindowUserPointer(window));
                handle->SendEvent(MouseMoveEvent(static_cast<int>(x), static_cast<int>(y)));
            });

        }

        ~MultiplatformWindow() override
        {
            glfwDestroyWindow(mHandle);
            glfwTerminate();
        }

        bool ShouldClose() const override
        {
            return glfwWindowShouldClose(mHandle);
        }

        void OnUpdate() override
        {
            glfwPollEvents();
        }

        void SetEventCallback(EventCallbackFn&& fn) override
        {
            mEventCallback = fn;
        }

        uint32_t GetWidth() const override { return mWidth; }
        uint32_t GetHeight() const override { return mHeight; }
        float GetAspect() const override { return static_cast<float>(mWidth) / static_cast<float>(mHeight); }

        Handle GetNativeHandle() const override { return mHandle; }
    };

    Scope<Window> Window::Create(const WindowDescription &description)
    {
        return CreateScope<MultiplatformWindow>(description);
    }
}