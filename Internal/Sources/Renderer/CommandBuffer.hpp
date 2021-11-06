#pragma once

#include "Core/Base.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Image.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/RenderPass.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/DescriptorSet.hpp"

namespace Fluent
{
    struct CommandBufferDescription
    {
        Handle device;
        Handle commandPool;
    };

    class CommandBuffer
    {
    protected:
        CommandBuffer() = default;
    public:
        virtual ~CommandBuffer() = default;

        virtual void Begin() const = 0;
        virtual void End() const = 0;

        virtual void BeginRenderPass(const Ref<RenderPass>& renderPass, const Ref<Framebuffer>& framebuffer) const = 0;
        virtual void EndRenderPass() const = 0;

        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const = 0;
        
        virtual void BindDescriptorSet(const Ref<Pipeline>& pipeline, const Ref<DescriptorSet>& set) const = 0;
        virtual void BindPipeline(const Ref<Pipeline>& pipeline) const = 0;

        virtual void BindVertexBuffer(const Ref<Buffer>& buffer, uint32_t offset) const = 0;
        virtual void BindIndexBuffer(const Ref<Buffer>& buffer, uint32_t offset, IndexType type) const = 0;

        virtual void SetScissor(uint32_t width, uint32_t height, int32_t x, int32_t y) = 0;
        virtual void SetViewport(uint32_t width, uint32_t height, float minDepth, float maxDepth, uint32_t x, uint32_t y) = 0;
        virtual void CopyBuffer(const Ref<Buffer>& src, uint32_t srcOffset, Buffer& dst, uint32_t dstOffset, uint32_t size) = 0;
        virtual void CopyBufferToImage(const Ref<Buffer>& src, uint32_t srcOffset, Image& dst, ImageUsage::Bits dstUsage) = 0;
        virtual void BlitImage(const Ref<Image>& src, ImageUsage::Bits srcUsage, const Ref<Image>& dst, ImageUsage::Bits dstUsage, Filter filter) const = 0;
        virtual void GenerateMipLevels(const Image& image, ImageUsage::Bits initialUsage, Filter filter) const = 0;
        
        virtual void ImageBarrier(Ref<Image>& image, ImageUsage::Bits src, ImageUsage::Bits dst) = 0;

        virtual Handle GetNativeHandle() const = 0;

        static Ref<CommandBuffer> Create(const CommandBufferDescription& description);
    };
} // namespace Fluent
