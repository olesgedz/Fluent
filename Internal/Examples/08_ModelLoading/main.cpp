#include <iostream>
#include <vector>
#include "Fluent/Fluent.hpp"

using namespace Fluent;

struct CameraUBO
{
    Matrix4 projection;
    Matrix4 view;
};

struct PushConstantBlock
{
    Matrix4 model;
    Vector4 viewPosition = Vector4(0.0, 0.0, 2.0, 0.0);
    Vector4 lightPosition = Vector4(1.0, 2.0, 2.0, 0.0);
};

class ParallaxMappingLayer : public Layer
{
private:
    Ref<Image>                  mRenderImage;
    Ref<Image>                  mDepthImage;
    Ref<RenderPass>             mRenderPass;
    Ref<Framebuffer>            mFramebuffer;
    Ref<Pipeline>               mPipeline;
    Ref<Buffer>                 mUniformBuffer;
    Ref<DescriptorSetLayout>    mDescriptorSetLayout;
    Ref<DescriptorSet>          mDescriptorSet;
    Ref<Sampler>                mSampler;

    Model                       mModel;

    Timer                       mTimer;

    CameraUBO                   mCameraUBO;
    PushConstantBlock           mPcb;
    Scope<UIContext>            mUIContext;
public:
    ParallaxMappingLayer() : Layer("VertexBuffer") {}

    void CreateUniformBuffer()
    {
        auto& window = Application::Get().GetWindow();

        mCameraUBO.projection   = CreatePerspectiveMatrix(Radians(45.0f), window->GetAspect(), 0, 100.f);
        mCameraUBO.view         = CreateLookAtMatrix(Vector3(0.0f, 0.0, 2.0f), Vector3(0.0, 0.0, -1.0), Vector3(0.0, 1.0, 0.0));

        BufferDescription bufferDesc{};
        bufferDesc.bufferUsage = BufferUsage::eUniformBuffer;
        bufferDesc.memoryUsage = MemoryUsage::eCpuToGpu;
        bufferDesc.size = sizeof(CameraUBO);

        mUniformBuffer = Buffer::Create(bufferDesc);
        mUniformBuffer->WriteData(&mCameraUBO, bufferDesc.size, 0);
    }

    void CreateSampler()
    {
        SamplerDescription samplerDesc{};
        samplerDesc.mipmapMode = SamplerMipmapMode::eLinear;
        samplerDesc.minLod = 0;
        samplerDesc.maxLod = 1000;
        mSampler = Sampler::Create(samplerDesc);
    }

    void OnAttach() override
    {
        FileSystem::SetShadersDirectory("../../../Internal/Examples/Shaders/");
        FileSystem::SetTexturesDirectory("../../../Internal/Examples/Textures/");
        FileSystem::SetModelsDirectory("../../../Internal/Examples/Models");

        auto& window = Application::Get().GetWindow();

        std::vector<ClearValue> clearValues(2);
        clearValues[0].color = Vector4(0.0, 0.0, 0.0, 1.0);
        clearValues[1].depth = 1.0f;
        clearValues[1].stencil = 0;

        RenderPassDescription renderPassDesc{};
        renderPassDesc.width = window->GetWidth();
        renderPassDesc.height = window->GetHeight();
        renderPassDesc.clearValues = clearValues;
        renderPassDesc.colorFormats = { Format::eR8G8B8A8Unorm };
        renderPassDesc.initialUsages = { ImageUsage::eUndefined, ImageUsage::eUndefined };
        renderPassDesc.finalUsages = { ImageUsage::eStorage, ImageUsage::eDepthStencilAttachment };
        renderPassDesc.attachmentLoadOps = { AttachmentLoadOp::eClear };
        renderPassDesc.depthStencilFormat = Format::eD32Sfloat;
        renderPassDesc.depthLoadOp = AttachmentLoadOp::eClear;
        renderPassDesc.stencilLoadOp = AttachmentLoadOp::eDontCare;
        renderPassDesc.sampleCount = SampleCount::e1;

        mRenderPass = RenderPass::Create(renderPassDesc);

        UIContextDescription uiDesc{};
        uiDesc.renderPass = mRenderPass;
        mUIContext = UIContext::Create(uiDesc);

        LoadModelDescription loadModelDescription{};
        loadModelDescription.filename = "backpack/backpack.obj";
        loadModelDescription.loadNormals = true;
        loadModelDescription.loadTexCoords = true;
        loadModelDescription.loadTangents = true;
        loadModelDescription.loadBitangents = true;

        ModelLoader modelLoader;
        mModel = modelLoader.Load(loadModelDescription);

        ShaderDescription vertexShaderDesc{};
        vertexShaderDesc.stage = ShaderStage::eVertex;
        vertexShaderDesc.filename = "08_ModelLoading/main.vert.glsl";

        ShaderDescription fragmentShaderDesc{};
        fragmentShaderDesc.stage = ShaderStage::eFragment;
        fragmentShaderDesc.filename = "08_ModelLoading/main.frag.glsl";

        auto vertexShader = Shader::Create(vertexShaderDesc);
        auto fragmentShader = Shader::Create(fragmentShaderDesc);
        
        DescriptorSetLayoutDescription descriptorSetLayoutDesc{};
        descriptorSetLayoutDesc.shaders = { vertexShader, fragmentShader };

        mDescriptorSetLayout = DescriptorSetLayout::Create(descriptorSetLayoutDesc);

        RasterizerStateDescription rasterizerState{};
        rasterizerState.cullMode = CullMode::eNone;
        rasterizerState.frontFace = FrontFace::eClockwise;

        DepthStateDescription depthStateDescription{};
        depthStateDescription.depthTest = true;
        depthStateDescription.depthWrite = true;
        depthStateDescription.compareOp = CompareOp::eLess;

        PipelineDescription pipelineDesc{};
        pipelineDesc.type = PipelineType::eGraphics;
        pipelineDesc.bindingDescriptions = modelLoader.GetVertexBindingDescription();
        pipelineDesc.attributeDescriptions = modelLoader.GetVertexAttributeDescription();

        pipelineDesc.descriptorSetLayout = mDescriptorSetLayout;
        pipelineDesc.rasterizerDescription = rasterizerState;
        pipelineDesc.depthStateDescription = depthStateDescription;
        pipelineDesc.renderPass = mRenderPass;

        mPipeline = Pipeline::Create(pipelineDesc);

        CreateUniformBuffer();
        CreateSampler();

        DescriptorSetDescription descriptorSetDesc{};
        descriptorSetDesc.descriptorSetLayout = mDescriptorSetLayout;
        mDescriptorSet = DescriptorSet::Create(descriptorSetDesc);

        std::vector<ImageUpdateDesc> imageUpdates(mModel.textures.size());
        for (uint32_t i = 0; i < imageUpdates.size(); ++i)
        {
            imageUpdates[i].image = mModel.textures[i];
            imageUpdates[i].usage = ImageUsage::eSampled;
        }

        BufferUpdateDesc bufferUpdateDesc{};
        bufferUpdateDesc.buffer = mUniformBuffer;
        bufferUpdateDesc.offset = 0;
        bufferUpdateDesc.range = sizeof(CameraUBO);

        std::vector<DescriptorSetUpdateDesc> updateDescriptions(3);
        updateDescriptions[0].binding = 0;
        updateDescriptions[0].bufferUpdates = { bufferUpdateDesc };
        updateDescriptions[0].descriptorType = DescriptorType::eUniformBuffer;

        ImageUpdateDesc imageUpdate {};
        imageUpdate.sampler = mSampler;
        updateDescriptions[1].binding = 1;
        updateDescriptions[1].imageUpdates = { imageUpdate };
        updateDescriptions[1].descriptorType = DescriptorType::eSampler;

        updateDescriptions[2].binding = 2;
        updateDescriptions[2].imageUpdates = imageUpdates;
        updateDescriptions[2].descriptorType = DescriptorType::eSampledImage;

        mDescriptorSet->UpdateDescriptorSet(updateDescriptions);
    }

    void OnDetach() override
    {
        mUniformBuffer = nullptr;
        mUIContext = nullptr;
        mRenderPass = nullptr;
        mPipeline = nullptr;
        mFramebuffer = nullptr;
        mRenderImage = nullptr;
    }

    void OnLoad() override
    {
        auto& window = Application::Get().GetWindow();

        ImageDescription imageDesc{};
        imageDesc.arraySize = 1;
        imageDesc.mipLevels = 1;
        imageDesc.depth = 1;
        imageDesc.format = Format::eR8G8B8A8Unorm;
        imageDesc.width = window->GetWidth();
        imageDesc.height = window->GetHeight();
        imageDesc.initialUsage = ImageUsage::Bits::eStorage;

        mRenderImage = Image::Create(imageDesc);

        imageDesc.format = Format::eD32Sfloat;
        imageDesc.initialUsage = ImageUsage::Bits::eDepthStencilAttachment;

        mDepthImage = Image::Create(imageDesc);

        FramebufferDescription framebufferDesc {};
        framebufferDesc.width = window->GetWidth();
        framebufferDesc.height = window->GetHeight();
        framebufferDesc.renderPass = mRenderPass;
        framebufferDesc.targets = {{ mRenderImage }};
        framebufferDesc.depthStencil = mDepthImage;

        mFramebuffer = Framebuffer::Create(framebufferDesc);

        mRenderPass->SetRenderArea(window->GetWidth(), window->GetHeight());

        mCameraUBO.projection = CreatePerspectiveMatrix(Radians(45.0f), window->GetAspect(), 0.1f, 100.0f);
    }

    void OnUnload() override
    {
        mFramebuffer = nullptr;
        mRenderImage = nullptr;
        mDepthImage = nullptr;
    }

    void OnUpdate(float deltaTime) override
    {
        mUniformBuffer->WriteData(&mCameraUBO, sizeof(CameraUBO), 0);

        auto& context = Application::Get().GetGraphicContext();
        auto& window = Application::Get().GetWindow();

        auto& cmd = context->GetCurrentCommandBuffer();
        cmd->BeginRenderPass(mRenderPass, mFramebuffer);
        cmd->SetViewport(window->GetWidth(), window->GetHeight(), 0.0f, 1.0f, 0, 0);
        cmd->SetScissor(window->GetWidth(), window->GetHeight(), 0, 0);
        cmd->BindDescriptorSet(mPipeline, mDescriptorSet);
        cmd->BindPipeline(mPipeline);
        mPcb.model = Matrix4(1.0);
        mPcb.model = glm::scale(mPcb.model, Vector3(0.4));
        mPcb.model = Rotate(mPcb.model, Radians(mTimer.Elapsed() * 10), Vector3(0.0, 1.0, 0.0));
        for (auto& mesh : mModel.meshes)
        {
            mPcb.model = mPcb.model * mesh.transform;
            cmd->PushConstants(mPipeline, 0, sizeof(PushConstantBlock), &mPcb);
            cmd->PushConstants(mPipeline, sizeof(Matrix4), sizeof(TextureIndices), &mesh.material.textureIndices);
            cmd->BindVertexBuffer(mesh.vertexBuffer, 0);
            cmd->BindIndexBuffer(mesh.indexBuffer, 0, IndexType::eUint32);
            cmd->DrawIndexed(mesh.indices.size(), 1, 0, 0, 0);
        }
        mUIContext->BeginFrame();
        ImGui::SliderFloat3("Light position", &mPcb.lightPosition.x, -5.0, 5.0);
        mUIContext->EndFrame();
        cmd->EndRenderPass();
        uint32_t activeImage = context->GetActiveImageIndex();
        auto swapchainImageUsage = context->GetSwapchainImageUsage(activeImage);
        auto swapchainImage = context->AcquireImage(activeImage, ImageUsage::eTransferDst);
        cmd->BlitImage(mRenderImage, ImageUsage::eStorage, swapchainImage, swapchainImageUsage, Filter::eLinear);
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
    ParallaxMappingLayer layer;
    app.PushLayer(layer);
    app.Run();

    return 0;
}
