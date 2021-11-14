
#pragma once

#include "Core/Base.hpp"
#include "Core/LayerStack.hpp"
#include "Core/Timer.hpp"
#include "Core/Window.hpp"

namespace Fluent
{
    class GraphicContext;
    
    struct ApplicationDescription
    {
        char** argv;
        WindowDescription windowDescription;
        bool askGraphicValidation;
    };

    class Application
    {
    private:
        static Application*     mApplication;
        Scope<Window>           mWindow;
        Scope<GraphicContext>   mGraphicContext;
        LayerStack              mLayerStack;
        Timer                   mDeltaTimer;

        bool                    mRunning = false;

        void OnEvent(const Event& event);
    public:
        explicit Application(const ApplicationDescription& description);
        ~Application();

        void PushLayer(Layer& layer);
        void PushOverlay(Layer& layer);
        void Run();

        Scope<GraphicContext>& GetGraphicContext();
        const Scope<Window>& GetWindow() const;
        static Application& Get();
    };
}