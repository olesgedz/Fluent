#pragma once

#include "Core/Base.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/DeviceAllocator.hpp"
#include "Renderer/Image.hpp"
#include "Renderer/CommandBuffer.hpp"
#include "Renderer/StagingBuffer.hpp"

namespace Fluent
{
    struct GraphicContextDescription
    {
        bool                    requestValidation;
        Handle                  window;
    };

    class GraphicContext
    {
    protected:
        GraphicContext() = default;
    public:
        virtual ~GraphicContext() = default;

        virtual void OnResize(uint32_t width, uint32_t height) = 0;

        virtual bool CanRender() const = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        virtual void WaitIdle() = 0;

        virtual Ref<RenderPass> GetDefaultRenderPass() const = 0;
        virtual Ref<Framebuffer> GetDefaultFramebuffer(uint32_t index) const = 0;

        virtual ImageUsage::Bits GetSwapchainImageUsage(uint32_t index) const = 0;
        virtual Ref<Image> AcquireImage(uint32_t imageIndex, ImageUsage::Bits usage) = 0;
        virtual void ImmediateSubmit(const Ref<CommandBuffer>& cmd) const = 0;
        
        virtual Handle              GetInstance() const = 0;
        virtual Handle              GetPhysicalDevice() const = 0;
        virtual Handle              GetDevice() = 0;
        virtual uint32_t            GetQueueIndex() = 0;
        virtual Handle              GetDeviceQueue() = 0;
        virtual DeviceAllocator&    GetDeviceAllocator() = 0;
        virtual Handle              GetCommandPool() = 0;
        virtual Handle              GetSwapchain() = 0;
        virtual Handle              GetDescriptorPool() const = 0;
        virtual uint32_t            GetPresentImageCount() const = 0;
        virtual uint32_t            GetActiveImageIndex() const = 0;
        virtual Ref<CommandBuffer>& GetCurrentCommandBuffer() = 0;
        virtual Ref<StagingBuffer>& GetStagingBuffer() = 0;

        static Scope<GraphicContext> Create(const GraphicContextDescription& description);
    };

    void SetGraphicContext(GraphicContext& context);
    GraphicContext& GetGraphicContext();
} // namespace Fluent
