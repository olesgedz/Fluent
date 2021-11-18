#include "Core/FileSystem.hpp"
#include "Core/Event.hpp"
#include "Core/Input.hpp"
#include "Renderer/GraphicContext.hpp"
#include "Core/Application.hpp"

namespace Fluent
{
    Application* Application::mApplication = nullptr;

    Application::Application(const ApplicationDescription& description)
    {
        if (!mApplication)
        {
            mApplication = this;
            FileSystem::Init(description.argv);
            mWindow = Window::Create(description.windowDescription);
            mWindow->SetEventCallback([this](const Event &event) { OnEvent(event); });
            mRunning = true;

            GraphicContextDescription gcontextDescription{};
            gcontextDescription.requestValidation = description.askGraphicValidation;
            gcontextDescription.window = mWindow->GetNativeHandle();

            mGraphicContext = GraphicContext::Create(gcontextDescription);
            /// Very important! You should do it right after creation
            SetGraphicContext(*mGraphicContext);
            mGraphicContext->OnResize(mWindow->GetWidth(), mWindow->GetHeight());
            Input::Init();
        }
    }

    Application::~Application()
    {
        if (mApplication)
        {
        }
    }

    void Application::PushLayer(Layer &layer)
    {
        mLayerStack.PushLayer(std::addressof(layer));
        layer.OnAttach();
        layer.OnLoad();
    }

    void Application::PushOverlay(Layer& layer)
    {
        mLayerStack.PushOverlay(std::addressof(layer));
        layer.OnAttach();
        layer.OnLoad();
    }

    void Application::Run()
    {
        while (mRunning)
        {
            float deltaTime = mDeltaTimer.Elapsed();
            mDeltaTimer.Reset();

            if (mGraphicContext->CanRender())
            {
                mGraphicContext->BeginFrame();
                for (auto layer : mLayerStack)
                    layer->OnUpdate(deltaTime);
                mGraphicContext->EndFrame();
            }

            Input::OnUpdate();
            mWindow->OnUpdate();
        }
        
        mGraphicContext->WaitIdle();

        for (auto layer : mLayerStack)
        {
            layer->OnUnload();
            layer->OnDetach();
        }
    }

    void Application::OnEvent(const Event &event)
    {
        switch (event.GetType())
        {
            case EventType::eWindowCloseEvent:
            {
                mRunning = false;
                break;
            }
            case EventType::eWindowResizeEvent:
            {
                mGraphicContext->OnResize(mWindow->GetWidth(), mWindow->GetHeight());
                for (auto& layer : mLayerStack)
                {
                    layer->OnUnload();
                    layer->OnLoad();
                }
                break;
            }
            default:
                break;
        }

        Input::OnEvent(event);
    }

    Scope<GraphicContext>& Application::GetGraphicContext() { return mGraphicContext; }
    const Scope<Window>& Application::GetWindow() const { return mWindow; }
    Application& Application::Get() { return *mApplication; }
}