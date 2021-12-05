#include "EditorLayer.hpp"
#include "Fluent/Fluent.hpp"

using namespace Fluent;

int main(int argc, char** argv)
{
    Log::SetLogLevel(Log::LogLevel::eTrace);

    WindowDescription windowDescription {};
    windowDescription.width = 800;
    windowDescription.height = 600;

    ApplicationDescription appDesc {};
    appDesc.argv = argv;
    appDesc.windowDescription = windowDescription;
    appDesc.askGraphicValidation = true;

    Application app(appDesc);

    EditorLayer layer;
    app.PushOverlay(layer);
    app.Run();
    return 0;
}