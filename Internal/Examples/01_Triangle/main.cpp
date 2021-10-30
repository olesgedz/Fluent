#include <iostream>
#include <filesystem>
#include "Fluent/Fluent.hpp"

using namespace Fluent;

class Triangle : public Layer
{
private:
    Ref<Image>          mImage;
    Ref<RenderPass>     mRenderPass;
    Ref<Framebuffer>    mFramebuffer;
    Ref<Pipeline>       mPipeline;
public:
    Triangle() : Layer("Triangle") {}

    void OnAttach() override
    {
        FileSystem::SetShadersDirectory("../../../Internal/Examples/Shaders/");

        auto& window = Application::Get().GetWindow();

        RenderPassDescription renderPassDesc{};
        renderPassDesc.width = window->GetWidth();
        renderPassDesc.height = window->GetHeight();
        renderPassDesc.clearValues = {{ 0.0, 0.0, 0.0 }};
        renderPassDesc.colorFormats = {{ Format::eR8G8B8A8Unorm }};
        renderPassDesc.initialUsages = {{ ImageUsage::eUndefined }};
        renderPassDesc.finalUsages = {{ ImageUsage::eStorage }};
        renderPassDesc.attachmentLoadOps = {{ AttachmentLoadOp::eClear }};
        renderPassDesc.sampleCount = SampleCount::e1;

        mRenderPass = RenderPass::Create(renderPassDesc);
        
        ShaderDescription vertexShaderDesc{};
        vertexShaderDesc.stage = ShaderStage::eVertex;
        vertexShaderDesc.filename = "triangle.vert.glsl";

        ShaderDescription fragmentShaderDesc{};
        fragmentShaderDesc.stage = ShaderStage::eFragment;
        fragmentShaderDesc.filename = "triangle.frag.glsl";

        auto vertexShader = Shader::Create(vertexShaderDesc);
        auto fragmentShader = Shader::Create(fragmentShaderDesc);
        
        RasterizerStateDescription rasterizerState{};
        rasterizerState.cullMode = CullMode::eNone;
        rasterizerState.frontFace = FrontFace::eCounterClockwise;

        PipelineDescription pipelineDesc{};
        pipelineDesc.type = PipelineType::eGraphics;
        pipelineDesc.shaders = { vertexShader, fragmentShader };
        pipelineDesc.rasterizerDescription = rasterizerState;
        pipelineDesc.renderPass = mRenderPass;

        mPipeline = Pipeline::Create(pipelineDesc);
    }

    void OnDetach() override
    {
        mPipeline = nullptr;
        mFramebuffer = nullptr;
        mImage = nullptr;
    }

    void OnLoad() override
    {
        auto& window = Application::Get().GetWindow();

        ImageDescription imageDesc {};
        imageDesc.arraySize = 1;
        imageDesc.mipLevels = 1;
        imageDesc.depth = 1;
        imageDesc.format = Format::eR8G8B8A8Unorm;
        imageDesc.width = window->GetWidth();
        imageDesc.height = window->GetHeight();
        imageDesc.initialUsage = ImageUsage::Bits::eStorage;

        mImage = Image::Create(imageDesc);

        FramebufferDescription framebufferDesc {};
        framebufferDesc.renderPass = mRenderPass;
        framebufferDesc.targets = {{ mImage }};

        mFramebuffer = Framebuffer::Create(framebufferDesc);

        mRenderPass->SetRenderArea(window->GetWidth(), window->GetHeight());
    }

    void OnUnload() override
    {
        mFramebuffer = nullptr;
        mImage = nullptr;
    }

    void OnUpdate(float deltaTime) override
    {
        auto& context = Application::Get().GetGraphicContext();
        auto& window = Application::Get().GetWindow();

        auto cmd = context->GetCurrentCommandBuffer();
        cmd->BeginRenderPass(mRenderPass, mFramebuffer);
        cmd->SetViewport(window->GetWidth(), window->GetHeight(), 0.0f, 1.0f, 0, 0);
        cmd->SetScissor(window->GetWidth(), window->GetHeight(), 0, 0);
        cmd->BindPipeline(mPipeline);
        cmd->Draw(3, 1, 0, 0);
        cmd->EndRenderPass();
        uint32_t activeImage = context->GetActiveImageIndex();
        auto swapchainImageUsage = context->GetSwapchainImageUsage(activeImage);
        auto swapchainImage = context->AcquireImage(activeImage, ImageUsage::eTransferDst);
        cmd->BlitImage(mImage, ImageUsage::eStorage, swapchainImage, swapchainImageUsage, Filter::eLinear);
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

    Application app(appDesc);
    Triangle triangle;
    app.PushLayer(triangle);
    app.Run();

    return 0;
}