#include <iostream>
#include <vector>
#include "Fluent/Fluent.hpp"

using namespace Fluent;

struct Vertex
{
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
};

struct CameraUBO
{
    Matrix4 projection;
    Matrix4 view;
    Matrix4 model;
};

struct PushConstantBlock
{
    float time;
    float mouseX;
    float mouseY;
};

class ComputeLayer : public Layer
{
private:
    Ref<Pipeline>               mPipeline;
    Ref<DescriptorSetLayout>    mDescriptorSetLayout;
    Ref<DescriptorSet>          mDescriptorSet;
    Ref<Image>                  mTexture;
    Timer                       mTimer;
public:
    ComputeLayer() : Layer("Compute") {}

    void CreateOutputTexture()
    {
        auto& window = Application::Get().GetWindow();
        ImageDescription imageDesc {};
        imageDesc.arraySize     = 1;
        imageDesc.mipLevels     = 1;
        imageDesc.depth         = 1;
        imageDesc.format        = Format::eR8G8B8A8Unorm;
        imageDesc.width         = 512;
        imageDesc.height        = 512;
        imageDesc.initialUsage  = ImageUsage::Bits::eStorage;

        mTexture = Image::Create(imageDesc);
    }

    void OnAttach() override
    {
        FileSystem::SetShadersDirectory("../../../Internal/Examples/Shaders/");
        FileSystem::SetTexturesDirectory("../../../Internal/Examples/Textures/");

        ShaderDescription computeShaderDesc{};
        computeShaderDesc.stage = ShaderStage::eCompute;
        computeShaderDesc.filename = "07_Compute/main.comp.glsl";

        auto computeShader = Shader::Create(computeShaderDesc);
        
        DescriptorSetLayoutDescription descriptorSetLayoutDesc{};
        descriptorSetLayoutDesc.shaders = { computeShader };

        mDescriptorSetLayout = DescriptorSetLayout::Create(descriptorSetLayoutDesc);

        PipelineDescription pipelineDesc{};
        pipelineDesc.type = PipelineType::eCompute;
        pipelineDesc.descriptorSetLayout = mDescriptorSetLayout;

        mPipeline = Pipeline::Create(pipelineDesc);

        CreateOutputTexture();

        DescriptorSetDescription descriptorSetDesc{};
        descriptorSetDesc.descriptorSetLayout = mDescriptorSetLayout;
        mDescriptorSet = DescriptorSet::Create(descriptorSetDesc);

        ImageUpdateDesc imageUpdateDesc{};
        imageUpdateDesc.image = mTexture;
        imageUpdateDesc.usage = ImageUsage::eStorage;

        std::vector<DescriptorSetUpdateDesc> updateDescriptions(1);
        updateDescriptions[0].binding = 0;
        updateDescriptions[0].imageUpdates = { imageUpdateDesc };
        updateDescriptions[0].descriptorType = DescriptorType::eStorageImage;

        mDescriptorSet->UpdateDescriptorSet(updateDescriptions);
    }

    void OnDetach() override
    {
        mTexture = nullptr;
        mPipeline = nullptr;
    }

    void OnLoad() override
    {
    }

    void OnUnload() override
    {

    }

    void OnUpdate(float deltaTime) override
    {
        auto& context = Application::Get().GetGraphicContext();
        auto cmd = context->GetCurrentCommandBuffer();
        cmd->BindDescriptorSet(mPipeline, mDescriptorSet);
        cmd->BindPipeline(mPipeline);
        PushConstantBlock pcb;
        pcb.time = mTimer.Elapsed();
        pcb.mouseX = Input::GetMouseX();
        pcb.mouseY = Input::GetMouseY();
        cmd->PushConstants(mPipeline, 0, sizeof(PushConstantBlock), &pcb);
        cmd->Dispatch(mTexture->GetWidth() / 16, mTexture->GetHeight() / 16, 1);
        uint32_t activeImage = context->GetActiveImageIndex();
        auto swapchainImageUsage = context->GetSwapchainImageUsage(activeImage);
        auto swapchainImage = context->AcquireImage(activeImage, ImageUsage::eTransferDst);
        cmd->BlitImage(mTexture, ImageUsage::eStorage, swapchainImage, swapchainImageUsage, Filter::eLinear);
        cmd->ImageBarrier(mTexture, ImageUsage::eTransferSrc, ImageUsage::eStorage);
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
    ComputeLayer layer;
    app.PushLayer(layer);
    app.Run();

    return 0;
}