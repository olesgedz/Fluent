#pragma once

#include <cstdint>
#include "Core/Base.hpp"
#include "Renderer/CommandBuffer.hpp"
#include "Renderer/StagingBuffer.hpp"

namespace Fluent
{
    struct VirtualFrameProviderDescription
    {
        uint32_t    frameCount;
        Handle      device;
        Handle      queue;
        Handle      commandPool;
        Handle      swapchain;
        uint32_t    swapchainImageCount;
        uint32_t    stagingBufferSize;
    };

    class VirtualFrameProvider
    {
    protected:
        VirtualFrameProvider() = default;
    public:
        virtual ~VirtualFrameProvider() = default;

        virtual bool BeginFrame() = 0;
        virtual bool EndFrame() = 0;

        virtual uint32_t GetActiveImageIndex() const = 0;
        virtual Ref<CommandBuffer>& GetCommandBuffer() = 0;
        virtual Ref<StagingBuffer>& GetStagingBuffer() = 0;

        static Scope<VirtualFrameProvider> Create(const VirtualFrameProviderDescription& description);
    };
} // namespace Fluent
