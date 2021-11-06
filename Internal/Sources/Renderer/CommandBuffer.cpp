#include "Renderer/CommandBuffer.hpp"

namespace Fluent
{
    class VulkanCommandBuffer : public CommandBuffer
    {
    private:
        vk::CommandBuffer mHandle;
    public:
        VulkanCommandBuffer(const CommandBufferDescription& description)
        {
            /// Create command buffers
            vk::CommandBufferAllocateInfo cmdAllocInfo;
            cmdAllocInfo
                    .setCommandPool((VkCommandPool)description.commandPool)
                    .setLevel(vk::CommandBufferLevel::ePrimary)
                    .setCommandBufferCount(1);
            
            vk::Device device = (VkDevice)description.device;
            mHandle = device.allocateCommandBuffers(cmdAllocInfo).front();
        }

        ~VulkanCommandBuffer() override = default;

        void Begin() const override
        {
            vk::CommandBufferBeginInfo beginInfo;
            beginInfo
                .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            mHandle.begin(beginInfo);
        }

        void End() const override
        {
            mHandle.end();
        }

        void BeginRenderPass(const Ref<RenderPass>& renderPass, const Ref<Framebuffer>& framebuffer) const override
        {
            std::vector<vk::ClearValue> clearValues(renderPass->GetClearValues().size());
            uint32_t i = 0;
            for (auto& clearValue : renderPass->GetClearValues())
            {
                clearValues[i]
                    .setColor(vk::ClearColorValue().setFloat32({ clearValue.color.r, clearValue.color.g, clearValue.color.b, clearValue.color.a }));
            }

            if (renderPass->HasDepthStencil())
                clearValues.back().setDepthStencil(vk::ClearDepthStencilValue().setDepth(renderPass->GetDepth()).setStencil(renderPass->GetStencil()));

            vk::Rect2D rect;
            rect
                .setExtent({ renderPass->GetWidth(), renderPass->GetHeight() })
                .setOffset({ 0, 0 });

            vk::RenderPassBeginInfo renderPassBeginInfo;
            renderPassBeginInfo
                    .setClearValues(clearValues)
                    .setRenderArea(rect)
                    .setFramebuffer((VkFramebuffer)framebuffer->GetNativeHandle())
                    .setRenderPass((VkRenderPass)renderPass->GetNativeHandle());

            mHandle.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        }

        void EndRenderPass() const override
        {
            mHandle.endRenderPass();
        }

        void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const override
        {
            mHandle.draw(vertexCount, instanceCount, firstVertex, firstInstance);
        }

        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const override
        {
            mHandle.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }
        
        void BindDescriptorSet(const Ref<Pipeline>& pipeline, const Ref<DescriptorSet>& set) const override
        {
            vk::PipelineLayout layout = (VkPipelineLayout)pipeline->GetPipelineLayout();
            vk::DescriptorSet nativeSet = (VkDescriptorSet)set->GetNativeHandle();
            mHandle.bindDescriptorSets(ToVulkanPipelineBindPoint(pipeline->GetType()), layout, 0, { nativeSet }, {});
        }

        void BindPipeline(const Ref<Pipeline>& pipeline) const override
        {
            mHandle.bindPipeline
            (
                ToVulkanPipelineBindPoint(pipeline->GetType()),
                (VkPipeline)pipeline->GetNativeHandle()
            );
        }

        void BindVertexBuffer(const Ref<Buffer>& buffer, uint32_t offset) const override
        {
            mHandle.bindVertexBuffers(0, vk::Buffer((VkBuffer)buffer->GetNativeHandle()), offset);
        }

        void BindIndexBuffer(const Ref<Buffer>& buffer, uint32_t offset, IndexType type) const override
        {
            mHandle.bindIndexBuffer((VkBuffer)buffer->GetNativeHandle(), offset, ToVulkanIndexType(type));
        }

        void SetScissor(uint32_t width, uint32_t height, int32_t x, int32_t y) override
        {
            vk::Rect2D scissor = vk::Rect2D()
                .setOffset({ x, y })
                .setExtent({ width, height });
            mHandle.setScissor(0, scissor);
        }

        void SetViewport(uint32_t width, uint32_t height, float minDepth, float maxDepth, uint32_t x, uint32_t y) override
        {
            vk::Viewport viewport = vk::Viewport()
                .setWidth(static_cast<float>(width))
                .setHeight(-static_cast<float>(height))
                .setMinDepth(minDepth)
                .setMaxDepth(maxDepth)
                .setX(static_cast<float>(x))
                .setY(static_cast<float>(y + height));

            mHandle.setViewport(0, viewport);
        }

        void CopyBuffer(const Ref<Buffer>& src, uint32_t srcOffset, Buffer& dst, uint32_t dstOffset, uint32_t size) override
        {
            vk::BufferCopy bufferCopy;
            bufferCopy
                .setSrcOffset(srcOffset)
                .setDstOffset(dstOffset)
                .setSize(size);

            mHandle.copyBuffer(vk::Buffer((VkBuffer)src->GetNativeHandle()), vk::Buffer((VkBuffer)dst.GetNativeHandle()), bufferCopy);
        }

        void CopyBufferToImage(const Ref<Buffer>& src, uint32_t srcOffset, Image& dst, ImageUsage::Bits dstUsage) override
        {
            if (dstUsage != ImageUsage::eTransferDst)
            {
                vk::ImageMemoryBarrier toTransferDstBarrier;
                toTransferDstBarrier
                    .setSrcAccessMask(ImageUsageToAccessFlags(dstUsage))
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setOldLayout(ImageUsageToImageLayout(dstUsage))
                    .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                    .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                    .setImage(vk::Image((VkImage)dst.GetNativeHandle()))
                    .setSubresourceRange(GetImageSubresourceRange(dst));

                mHandle.pipelineBarrier
                (
                    ImageUsageToPipelineStage(dstUsage),
                    vk::PipelineStageFlagBits::eTransfer,
                    {},
                    {},
                    {},
                    toTransferDstBarrier
                );
            }

            auto dstLayers = GetImageSubresourceLayers(dst);

            vk::BufferImageCopy bufferToImageCopyInfo;
            bufferToImageCopyInfo
                .setBufferOffset(srcOffset)
                .setBufferImageHeight(0)
                .setBufferRowLength(0)
                .setImageSubresource(dstLayers)
                .setImageOffset(vk::Offset3D{ 0, 0, 0 })
                .setImageExtent(vk::Extent3D{
                        dst.GetWidth(),
                        dst.GetHeight(),
                        1
                });

            mHandle.copyBufferToImage(vk::Buffer((VkBuffer)src->GetNativeHandle()), vk::Image((VkImage)dst.GetNativeHandle()), ImageUsageToImageLayout(ImageUsage::eTransferDst), bufferToImageCopyInfo);
        }

        void BlitImage(const Ref<Image>& src, ImageUsage::Bits srcUsage, const Ref<Image>& dst, ImageUsage::Bits dstUsage, Filter filter) const override
        {
            auto sourceRange = GetImageSubresourceRange(*src);
            auto distanceRange = GetImageSubresourceRange(*dst);

            std::array<vk::ImageMemoryBarrier, 2> barriers;
            size_t barrierCount = 0;

            vk::ImageMemoryBarrier toTransferSrcBarrier;
            toTransferSrcBarrier
                    .setSrcAccessMask(ImageUsageToAccessFlags(srcUsage))
                    .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
                    .setOldLayout(ImageUsageToImageLayout(srcUsage))
                    .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                    .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                    .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                    .setImage(vk::Image((VkImage)src->GetNativeHandle()))
                    .setSubresourceRange(sourceRange);

            vk::ImageMemoryBarrier toTransferDstBarrier;
            toTransferDstBarrier
                    .setSrcAccessMask(ImageUsageToAccessFlags(dstUsage))
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setOldLayout(ImageUsageToImageLayout(dstUsage))
                    .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                    .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                    .setImage(vk::Image((VkImage)dst->GetNativeHandle()))
                    .setSubresourceRange(distanceRange);

            if (srcUsage != ImageUsage::eTransferSrc)
                barriers[barrierCount++] = toTransferSrcBarrier;
            if (dstUsage != ImageUsage::eTransferDst)
                barriers[barrierCount++] = toTransferDstBarrier;

            if (barrierCount > 0)
            {
                mHandle.pipelineBarrier(
                    ImageUsageToPipelineStage(srcUsage) | ImageUsageToPipelineStage(dstUsage),
                    vk::PipelineStageFlagBits::eTransfer,
                    { }, // dependency flags
                    0, nullptr, // memory barriers
                    0, nullptr, // buffer barriers
                    static_cast<uint32_t>(barrierCount), barriers.data()
                );
            }

            auto sourceLayers = GetImageSubresourceLayers(*src);
            auto distanceLayers = GetImageSubresourceLayers(*dst);

            vk::ImageBlit imageBlitInfo;
            imageBlitInfo
                .setSrcOffsets
                ({
                    vk::Offset3D{ 0, 0, 0 },
                    vk::Offset3D{ (int32_t)src->GetWidth(), (int32_t)src->GetHeight(), 1 }
                })
                .setDstOffsets
                ({
                    vk::Offset3D{ 0, 0, 0 },
                    vk::Offset3D{ (int32_t)dst->GetWidth(), (int32_t)dst->GetHeight(), 1 }
                })
                .setSrcSubresource(sourceLayers)
                .setDstSubresource(distanceLayers);

            mHandle.blitImage(
                vk::Image((VkImage)src->GetNativeHandle()),
                vk::ImageLayout::eTransferSrcOptimal,
                vk::Image((VkImage)dst->GetNativeHandle()),
                vk::ImageLayout::eTransferDstOptimal,
                imageBlitInfo,
                static_cast<vk::Filter>(filter)
            );
        }

        void ImageBarrier(Ref<Image>& image, ImageUsage::Bits src, ImageUsage::Bits dst) override
        {
            if (src == dst) return;

            vk::ImageSubresourceRange imageSubresourceRange = GetImageSubresourceRange(*image);

            vk::ImageMemoryBarrier barrier;
            barrier
                .setSrcAccessMask(ImageUsageToAccessFlags(src))
                .setDstAccessMask(ImageUsageToAccessFlags(dst))
                .setOldLayout(ImageUsageToImageLayout(src))
                .setNewLayout(ImageUsageToImageLayout(dst))
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setImage((VkImage)image->GetNativeHandle())
                .setSubresourceRange(imageSubresourceRange);

            mHandle.pipelineBarrier
            (
                ImageUsageToPipelineStage(src),
                ImageUsageToPipelineStage(dst),
                {}, {}, {},
                barrier
            );
        }

        void GenerateMipLevels(const Image& image, ImageUsage::Bits initialUsage, Filter filter) const override
        {
            if (image.GetMipLevelsCount() < 2) return;

            auto sourceRange = GetImageSubresourceRange(image);
            auto distanceRange = GetImageSubresourceRange(image);
            auto sourceLayers = GetImageSubresourceLayers(image);
            auto distanceLayers = GetImageSubresourceLayers(image);
            auto sourceUsage = initialUsage;
            uint32_t sourceWidth = image.GetWidth();
            uint32_t sourceHeight = image.GetHeight();
            uint32_t destinationWidth = image.GetWidth();
            uint32_t destinationHeight = image.GetHeight();

            for (size_t i = 0; i + 1 < image.GetMipLevelsCount(); i++)
            {
                sourceWidth = destinationWidth;
                sourceHeight = destinationHeight;
                destinationWidth = std::max(sourceWidth / 2, 1u);
                destinationHeight = std::max(sourceHeight / 2, 1u);
                
                sourceLayers.setMipLevel(i);
                sourceRange.setBaseMipLevel(i);
                sourceRange.setLevelCount(1);

                distanceLayers.setMipLevel(i + 1);
                distanceRange.setBaseMipLevel(i + 1);
                distanceRange.setLevelCount(1);

                std::array<vk::ImageMemoryBarrier, 2> imageBarriers;
                imageBarriers[0] // to transfer source
                    .setSrcAccessMask(ImageUsageToAccessFlags(sourceUsage))
                    .setDstAccessMask(ImageUsageToAccessFlags(ImageUsage::eTransferSrc))
                    .setOldLayout(ImageUsageToImageLayout(sourceUsage))
                    .setNewLayout(ImageUsageToImageLayout(ImageUsage::eTransferSrc))
                    .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                    .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                    .setImage((VkImage)image.GetNativeHandle())
                    .setSubresourceRange(sourceRange);

                imageBarriers[1] // to transfer distance
                    .setSrcAccessMask(ImageUsageToAccessFlags(ImageUsage::eUndefined))
                    .setDstAccessMask(ImageUsageToAccessFlags(ImageUsage::eTransferDst))
                    .setOldLayout(ImageUsageToImageLayout(ImageUsage::eUndefined))
                    .setNewLayout(ImageUsageToImageLayout(ImageUsage::eTransferDst))
                    .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                    .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                    .setImage((VkImage)image.GetNativeHandle())
                    .setSubresourceRange(distanceRange);

                mHandle.pipelineBarrier(
                    vk::PipelineStageFlagBits::eTransfer,
                    vk::PipelineStageFlagBits::eTransfer,
                    { }, // dependencies
                    { }, // memory barriers
                    { }, // buffer barriers,
                    imageBarriers
                );
                sourceUsage = ImageUsage::eTransferDst;

                vk::ImageBlit imageBlitInfo;
                imageBlitInfo
                    .setSrcOffsets({
                        vk::Offset3D{ 0, 0, 0 },
                        vk::Offset3D{ (int32_t)sourceWidth, (int32_t)sourceHeight, 1 }
                    })
                    .setDstOffsets({
                        vk::Offset3D{ 0, 0, 0 },
                        vk::Offset3D{ (int32_t)destinationWidth, (int32_t)destinationHeight, 1 }
                    })
                    .setSrcSubresource(sourceLayers)
                    .setDstSubresource(distanceLayers);

                mHandle.blitImage(
                    (VkImage)image.GetNativeHandle(),
                    vk::ImageLayout::eTransferSrcOptimal,
                    (VkImage)image.GetNativeHandle(),
                    vk::ImageLayout::eTransferDstOptimal,
                    imageBlitInfo,
                    ToVulkanFilter(filter)
                );

            }

            auto mipLevelsSubresourceRange = GetImageSubresourceRange(image);
            mipLevelsSubresourceRange.setLevelCount(mipLevelsSubresourceRange.levelCount - 1);
            vk::ImageMemoryBarrier mipLevelsTransfer;
            mipLevelsTransfer
                .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
                .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setImage((VkImage)image.GetNativeHandle())
                .setSubresourceRange(mipLevelsSubresourceRange);

            mHandle.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eTransfer,
                { }, // dependecies
                { }, // memory barriers
                { }, // buffer barriers
                mipLevelsTransfer
            );
        }

        Handle GetNativeHandle() const override
        {
            return mHandle;
        }
    };

    /// Interface

    Ref<CommandBuffer> CommandBuffer::Create(const CommandBufferDescription& description)
    {
        return CreateRef<VulkanCommandBuffer>(description);
    }
} // namespace Fluent
