#include "Fluent/Fluent.hpp"

using namespace Fluent;

class ImGuiLayer : public Layer
{
private:
    Ref<Image>          mImage;
    Ref<RenderPass>     mRenderPass;
    Ref<Framebuffer>    mFramebuffer;
    Scope<UIContext>    mUIContext;
public:
    ImGuiLayer() : Layer("ImGui") {}
    ~ImGuiLayer() = default;

    void OnAttach()
    {
        auto& window = Application::Get().GetWindow();

        ClearValue clearValue{};
        clearValue.color = Vector4(0.0, 0.0, 0.0, 1.0);
        RenderPassDescription renderPassDesc{};
        renderPassDesc.width = window->GetWidth();
        renderPassDesc.height = window->GetHeight();
        renderPassDesc.clearValues = { clearValue };
        renderPassDesc.colorFormats = { Format::eR8G8B8A8Unorm };
        renderPassDesc.initialUsages = { ImageUsage::eUndefined };
        renderPassDesc.finalUsages = { ImageUsage::eSampled };
        renderPassDesc.attachmentLoadOps = { AttachmentLoadOp::eClear };
        renderPassDesc.sampleCount = SampleCount::e1;

        mRenderPass = RenderPass::Create(renderPassDesc);

        UIContextDescription uiDesc{};
        uiDesc.renderPass = mRenderPass;
        mUIContext = UIContext::Create(uiDesc);
    }

    void OnLoad()
    {
        auto& window = Application::Get().GetWindow();

        ImageDescription imageDesc {};
        imageDesc.arraySize = 1;
        imageDesc.mipLevels = 1;
        imageDesc.depth = 1;
        imageDesc.format = Format::eR8G8B8A8Unorm;
        imageDesc.width = window->GetWidth();
        imageDesc.height = window->GetHeight();
        imageDesc.initialUsage = ImageUsage::Bits::eSampled;

        mImage = Image::Create(imageDesc);

        FramebufferDescription framebufferDesc {};
        framebufferDesc.width = window->GetWidth();
        framebufferDesc.height = window->GetHeight();
        framebufferDesc.renderPass = mRenderPass;
        framebufferDesc.targets = { mImage };

        mFramebuffer = Framebuffer::Create(framebufferDesc);

        mRenderPass->SetRenderArea(window->GetWidth(), window->GetHeight());
    }

    void OnUnload()
    {
        mFramebuffer = nullptr;
        mImage = nullptr;
    }

    void OnDetach()
    {
        mUIContext = nullptr;
        mRenderPass = nullptr;
    }

    void OnUpdate(float deltaTime)
    {
        auto& context = GetGraphicContext();
        auto& cmd = context.GetCurrentCommandBuffer();
        auto& window = Application::Get().GetWindow();

        cmd->BeginRenderPass(mRenderPass, mFramebuffer);
        cmd->SetViewport(window->GetWidth(), window->GetHeight(), 0.0f, 1.0f, 0, 0);
        cmd->SetScissor(window->GetWidth(), window->GetHeight(), 0, 0);
        mUIContext->BeginFrame();
        ImGui::ShowDemoWindow();
        mUIContext->EndFrame();
        cmd->EndRenderPass();
        uint32_t activeImage = context.GetActiveImageIndex();
        auto swapchainImageUsage = context.GetSwapchainImageUsage(activeImage);
        auto swapchainImage = context.AcquireImage(activeImage, ImageUsage::eTransferDst);
        cmd->BlitImage(mImage, ImageUsage::eSampled, swapchainImage, swapchainImageUsage, Filter::eLinear);
    }
};

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

    ImGuiLayer layer;
    app.PushLayer(layer);
    app.Run();
    return 0;
}