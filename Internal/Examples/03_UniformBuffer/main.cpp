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

static Vector3 cubeCenter = Vector3(0.25, 0.25, -0.25);

static std::vector<Vertex> cubeVertices
{
    // FRONT
    {{ 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }}, // 0
    {{ 0.0f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }}, // 1
    {{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }}, // 2
    {{ 0.5f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }}, // 3

    // RIGHT
    {{ 0.5f, 0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }}, // 4
    {{ 0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }}, // 5
    {{ 0.5f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }}, // 6
    {{ 0.5f, 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }}, // 7

    // BACK
    {{ 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }}, // 8
    {{ 0.5f, 0.0f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }}, // 9
    {{ 0.0f, 0.0f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }}, // 10
    {{ 0.0f, 0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }}, // 11

    // LEFT
    {{ 0.0f, 0.5f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }}, // 12
    {{ 0.0f, 0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }}, // 13
    {{ 0.0f, 0.0f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }}, // 14
    {{ 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }}, // 15

    // UP
    {{ 0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }}, // 16
    {{ 0.0f, 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }}, // 17
    {{ 0.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }}, // 18
    {{ 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }}, // 19

    // DOWN
    {{ 0.5f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }}, // 20
    {{ 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }}, // 21
    {{ 0.0f, 0.0f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }}, // 22
    {{ 0.5f, 0.0f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }}, // 23
};

static std::vector<uint32_t> indices
{
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20
};

class VertexBufferLayer : public Layer
{
private:
    Ref<Image>                  mImage;
    Ref<RenderPass>             mRenderPass;
    Ref<Framebuffer>            mFramebuffer;
    Ref<Pipeline>               mPipeline;
    Ref<Buffer>                 mVertexBuffer;
    Ref<Buffer>                 mIndexBuffer;
    Ref<Buffer>                 mUniformBuffer;
    Ref<DescriptorSetLayout>    mDescriptorSetLayout;
    Ref<DescriptorSet>          mDescriptorSet;
    
    Timer                       mTimer;

    CameraUBO                   mCameraUBO;
public:
    VertexBufferLayer() : Layer("VertexBuffer") {}

    void CreateVertexBuffer()
    {
        BufferDescription bufferDesc{};
        bufferDesc.bufferUsage = BufferUsage::eVertexBuffer;
        bufferDesc.memoryUsage = MemoryUsage::eCpuToGpu;
        bufferDesc.size = cubeVertices.size() * sizeof(cubeVertices[0]);

        mVertexBuffer = Buffer::Create(bufferDesc);
        mVertexBuffer->WriteData(cubeVertices.data(), bufferDesc.size, 0);
    }

    void CreateIndexBuffer()
    {
        BufferDescription bufferDesc{};
        bufferDesc.bufferUsage = BufferUsage::eIndexBuffer;
        bufferDesc.memoryUsage = MemoryUsage::eCpuToGpu;
        bufferDesc.size = indices.size() * sizeof(indices[0]);

        mIndexBuffer = Buffer::Create(bufferDesc);
        mIndexBuffer->WriteData(indices.data(), bufferDesc.size, 0);
    }

    void CreateUniformBuffer()
    {
        auto& window = Application::Get().GetWindow();

        mCameraUBO.projection   = CreatePerspectiveMatrix(Radians(45.0f), window->GetAspect(), 0, 100.f);
        mCameraUBO.view         = CreateLookAtMatrix(Vector3(0.0f, 0.0, 2.0f), Vector3(0.0, 0.0, -1.0), Vector3(0.0, 1.0, 0.0));
        mCameraUBO.model        = Matrix4(1.0f);

        BufferDescription bufferDesc{};
        bufferDesc.bufferUsage = BufferUsage::eUniformBuffer;
        bufferDesc.memoryUsage = MemoryUsage::eCpuToGpu;
        bufferDesc.size = sizeof(CameraUBO);

        mUniformBuffer = Buffer::Create(bufferDesc);
        mUniformBuffer->WriteData(&mCameraUBO, bufferDesc.size, 0);
    }

    void OnAttach() override
    {
        FileSystem::SetShadersDirectory("../../../Internal/Examples/Shaders/");

        auto& window = Application::Get().GetWindow();

        ClearValue clearValue{};
        clearValue.color = Vector4(0.0, 0.0, 0.0, 1.0);

        RenderPassDescription renderPassDesc{};
        renderPassDesc.width = window->GetWidth();
        renderPassDesc.height = window->GetHeight();
        renderPassDesc.clearValues = { clearValue };
        renderPassDesc.colorFormats = { Format::eR8G8B8A8Unorm };
        renderPassDesc.initialUsages = { ImageUsage::eUndefined };
        renderPassDesc.finalUsages = { ImageUsage::eStorage };
        renderPassDesc.attachmentLoadOps = { AttachmentLoadOp::eClear };
        renderPassDesc.sampleCount = SampleCount::e1;

        mRenderPass = RenderPass::Create(renderPassDesc);
        
        ShaderDescription vertexShaderDesc{};
        vertexShaderDesc.stage = ShaderStage::eVertex;
        vertexShaderDesc.filename = "03_UniformBuffer/main.vert.glsl";

        ShaderDescription fragmentShaderDesc{};
        fragmentShaderDesc.stage = ShaderStage::eFragment;
        fragmentShaderDesc.filename = "03_UniformBuffer/main.frag.glsl";

        auto vertexShader = Shader::Create(vertexShaderDesc);
        auto fragmentShader = Shader::Create(fragmentShaderDesc);
        
        DescriptorSetLayoutDescription descriptorSetLayoutDesc{};
        descriptorSetLayoutDesc.shaders = { vertexShader, fragmentShader };

        mDescriptorSetLayout = DescriptorSetLayout::Create(descriptorSetLayoutDesc);

        RasterizerStateDescription rasterizerState{};
        rasterizerState.cullMode = CullMode::eBack;
        rasterizerState.frontFace = FrontFace::eCounterClockwise;

        PipelineDescription pipelineDesc{};
        pipelineDesc.type = PipelineType::eGraphics;
        pipelineDesc.bindingDescriptions =
        {
            VertexBindingDescription
            {
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = VertexInputRate::eVertex
            }
        };

        pipelineDesc.attributeDescriptions = 
        {
            VertexAttributeDescription
            {
                .location = 0,
                .binding = 0,
                .format = Format::eR32G32B32Sfloat,
                .offset = offsetof(Vertex, position)
            },
            {
                .location = 1,
                .binding = 0,
                .format = Format::eR32G32B32Sfloat,
                .offset = offsetof(Vertex, normal)
            },
            {
                .location = 2,
                .binding = 0,
                .format = Format::eR32G32Sfloat,
                .offset = offsetof(Vertex, uv)
            }
        };

        pipelineDesc.descriptorSetLayout = mDescriptorSetLayout;
        pipelineDesc.rasterizerDescription = rasterizerState;
        pipelineDesc.renderPass = mRenderPass;

        mPipeline = Pipeline::Create(pipelineDesc);

        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffer();

        DescriptorSetDescription descriptorSetDesc{};
        descriptorSetDesc.descriptorSetLayout = mDescriptorSetLayout;
        mDescriptorSet = DescriptorSet::Create(descriptorSetDesc);

        DescriptorUpdateDesc updateDesc{};
        updateDesc.binding = 0;
        updateDesc.bufferUpdate.buffer = mUniformBuffer;
        updateDesc.bufferUpdate.offset = 0;
        updateDesc.bufferUpdate.range = sizeof(CameraUBO);
        updateDesc.descriptorType = DescriptorType::eUniformBuffer;
        mDescriptorSet->UpdateDescriptorSet(updateDesc);
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
        mUniformBuffer = nullptr;
        mIndexBuffer = nullptr;
        mVertexBuffer = nullptr;
        mFramebuffer = nullptr;
        mImage = nullptr;
    }

    void OnUpdate(float deltaTime) override
    {
        mCameraUBO.model = Rotate(Matrix4(1.0), mTimer.Elapsed(), Vector3(0.0, 1.0, 0.0));
        mCameraUBO.model = Translate(mCameraUBO.model, -cubeCenter);
        mUniformBuffer->WriteData(&mCameraUBO, sizeof(CameraUBO), 0);

        auto& context = Application::Get().GetGraphicContext();
        auto& window = Application::Get().GetWindow();

        auto cmd = context->GetCurrentCommandBuffer();
        cmd->BeginRenderPass(mRenderPass, mFramebuffer);
        cmd->SetViewport(window->GetWidth(), window->GetHeight(), 0.0f, 1.0f, 0, 0);
        cmd->SetScissor(window->GetWidth(), window->GetHeight(), 0, 0);
        cmd->BindDescriptorSet(mPipeline, mDescriptorSet);
        cmd->BindPipeline(mPipeline);
        cmd->BindVertexBuffer(mVertexBuffer, 0);
        cmd->BindIndexBuffer(mIndexBuffer, 0, IndexType::eUint32);
        cmd->DrawIndexed(indices.size(), 1, 0, 0, 0);
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
    VertexBufferLayer layer;
    app.PushLayer(layer);
    app.Run();

    return 0;
}