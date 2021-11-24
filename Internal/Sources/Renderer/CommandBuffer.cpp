#include "Renderer/CommandBuffer.hpp"

namespace Fluent
{
    class VulkanCommandBuffer : public CommandBuffer
    {
    private:
        VkCommandBuffer mHandle;
    public:
        VulkanCommandBuffer(const CommandBufferDescription& description)
        {
            /// Create command buffers
            VkCommandBufferAllocateInfo cmdAllocInfo{};
            cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmdAllocInfo.commandPool = (VkCommandPool)description.commandPool;
            cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmdAllocInfo.commandBufferCount = 1;

            VkDevice device = (VkDevice)description.device;
            vkAllocateCommandBuffers(device, &cmdAllocInfo, &mHandle);
        }

        ~VulkanCommandBuffer() override = default;

        void Begin() const override
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(mHandle, &beginInfo);
        }

        void End() const override
        {
            vkEndCommandBuffer(mHandle);
        }

        void BeginRenderPass(const Ref<RenderPass>& renderPass, const Ref<Framebuffer>& framebuffer) const override
        {
            std::vector<VkClearValue> clearValues(renderPass->GetClearValues().size()
                                                    + (renderPass->HasDepthStencil() ? 1 : 0));
            uint32_t i = 0;
            for (auto& clearValue : renderPass->GetClearValues())
            {
                auto& vkClearValue = clearValues[i].color.float32;
                vkClearValue[0] = clearValue.color.r;
                vkClearValue[1] = clearValue.color.g;
                vkClearValue[2] = clearValue.color.b;
                vkClearValue[3] = clearValue.color.a;
            }

            if (renderPass->HasDepthStencil())
            {
                clearValues.back().depthStencil = { renderPass->GetDepth(), renderPass->GetStencil() };
            }

            VkRect2D rect{};
            rect.extent = { renderPass->GetWidth(), renderPass->GetHeight() };
            rect.offset = { 0, 0 };

            VkRenderPassBeginInfo renderPassBeginInfo{};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.clearValueCount = clearValues.size();
            renderPassBeginInfo.pClearValues = clearValues.data();
            renderPassBeginInfo.renderArea = rect;
            renderPassBeginInfo.framebuffer = (VkFramebuffer)framebuffer->GetNativeHandle();
            renderPassBeginInfo.renderPass = (VkRenderPass)renderPass->GetNativeHandle();

            vkCmdBeginRenderPass(mHandle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        }

        void EndRenderPass() const override
        {
            vkCmdEndRenderPass(mHandle);
        }

        void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const override
        {
            vkCmdDispatch(mHandle, groupCountX, groupCountY, groupCountZ);
        }

        void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const override
        {
            vkCmdDraw(mHandle, vertexCount, instanceCount, firstVertex, firstInstance);
        }

        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const override
        {
            vkCmdDrawIndexed(mHandle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }
        
        void BindDescriptorSet(const Ref<Pipeline>& pipeline, const Ref<DescriptorSet>& set) const override
        {
            VkPipelineLayout layout = (VkPipelineLayout)pipeline->GetPipelineLayout();
            VkDescriptorSet nativeSet = (VkDescriptorSet)set->GetNativeHandle();
            vkCmdBindDescriptorSets
            (
                mHandle,
                ToVulkanPipelineBindPoint(pipeline->GetType()),
                layout,
                0, 1, &nativeSet,
                0, nullptr
            );
        }

        void BindPipeline(const Ref<Pipeline>& pipeline) const override
        {
            vkCmdBindPipeline(mHandle, ToVulkanPipelineBindPoint(pipeline->GetType()), (VkPipeline)pipeline->GetNativeHandle());
        }

        void BindVertexBuffer(const Ref<Buffer>& buffer, uint32_t offset) const override
        {
            VkBuffer vkBuffer = (VkBuffer)buffer->GetNativeHandle();
            VkDeviceSize vkOffsets = offset;
            vkCmdBindVertexBuffers(mHandle, 0, 1, &vkBuffer, &vkOffsets);
        }

        void BindIndexBuffer(const Ref<Buffer>& buffer, uint32_t offset, IndexType type) const override
        {
            vkCmdBindIndexBuffer(mHandle, (VkBuffer)buffer->GetNativeHandle(), offset, ToVulkanIndexType(type));
        }

        void SetScissor(uint32_t width, uint32_t height, int32_t x, int32_t y) override
        {
            VkRect2D scissor{};
            scissor.offset = { x, y };
            scissor.extent = { width, height };
            vkCmdSetScissor(mHandle, 0, 1, &scissor);
        }

        void SetViewport(uint32_t width, uint32_t height, float minDepth, float maxDepth, uint32_t x, uint32_t y) override
        {
            VkViewport viewport{};
            viewport.width = static_cast<float>(width);
            viewport.height = -static_cast<float>(height);
            viewport.minDepth = minDepth;
            viewport.maxDepth = maxDepth;
            viewport.x = static_cast<float>(x);
            viewport.y = static_cast<float>(y + height);

            vkCmdSetViewport(mHandle, 0, 1, &viewport);
        }

        void PushConstants(const Ref<Pipeline>& pipeline, uint32_t offset, uint32_t size, const void* data) const override
        {
            vkCmdPushConstants
            (
               mHandle,
               (VkPipelineLayout)pipeline->GetPipelineLayout(),
               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
               offset, size, data
            );
        }

        void CopyBuffer(const Ref<Buffer>& src, uint32_t srcOffset, Buffer& dst, uint32_t dstOffset, uint32_t size) override
        {
            VkBufferCopy bufferCopy{};
            bufferCopy.srcOffset = srcOffset;
            bufferCopy.dstOffset = dstOffset;
            bufferCopy.size = size;

            vkCmdCopyBuffer(mHandle, (VkBuffer)src->GetNativeHandle(), (VkBuffer)dst.GetNativeHandle(), 1, &bufferCopy);
        }

        void CopyBufferToImage(const Ref<Buffer>& src, uint32_t srcOffset, Image& dst, ImageUsage::Bits dstUsage) override
        {
            if (dstUsage != ImageUsage::eTransferDst)
            {
                VkImageMemoryBarrier toTransferDstBarrier{};
                toTransferDstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                toTransferDstBarrier.srcAccessMask = ImageUsageToAccessFlags(dstUsage);
                toTransferDstBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                toTransferDstBarrier.oldLayout = ImageUsageToImageLayout(dstUsage);
                toTransferDstBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                toTransferDstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                toTransferDstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                toTransferDstBarrier.image = (VkImage)dst.GetNativeHandle();
                toTransferDstBarrier.subresourceRange = GetImageSubresourceRange(dst);

                vkCmdPipelineBarrier
                (
                    mHandle,
                    ImageUsageToPipelineStage(dstUsage),
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &toTransferDstBarrier
                );
            }

            auto dstLayers = GetImageSubresourceLayers(dst);

            VkBufferImageCopy bufferToImageCopyInfo{};
            bufferToImageCopyInfo.bufferOffset = srcOffset;
            bufferToImageCopyInfo.bufferImageHeight = 0;
            bufferToImageCopyInfo.bufferRowLength = 0;
            bufferToImageCopyInfo.imageSubresource = dstLayers;
            bufferToImageCopyInfo.imageOffset = VkOffset3D{ 0, 0, 0 };
            bufferToImageCopyInfo.imageExtent = VkExtent3D { dst.GetWidth(), dst.GetHeight(), 1 };

            vkCmdCopyBufferToImage
            (
                mHandle,
                (VkBuffer)src->GetNativeHandle(), (VkImage)dst.GetNativeHandle(),
                ImageUsageToImageLayout(ImageUsage::eTransferDst),
                1,
                &bufferToImageCopyInfo
            );
        }

        void BlitImage(const Ref<Image>& src, ImageUsage::Bits srcUsage, const Ref<Image>& dst, ImageUsage::Bits dstUsage, Filter filter) const override
        {
            auto sourceRange = GetImageSubresourceRange(*src);
            auto distanceRange = GetImageSubresourceRange(*dst);

            VkImageMemoryBarrier barriers[2] = {};
            size_t barrierCount = 0;

            VkImageMemoryBarrier toTransferSrcBarrier{};
            toTransferSrcBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            toTransferSrcBarrier.srcAccessMask = ImageUsageToAccessFlags(srcUsage);
            toTransferSrcBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            toTransferSrcBarrier.oldLayout = ImageUsageToImageLayout(srcUsage);
            toTransferSrcBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            toTransferSrcBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toTransferSrcBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toTransferSrcBarrier.image = (VkImage)src->GetNativeHandle();
            toTransferSrcBarrier.subresourceRange = sourceRange;

            VkImageMemoryBarrier toTransferDstBarrier{};
            toTransferDstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            toTransferDstBarrier.srcAccessMask = ImageUsageToAccessFlags(dstUsage);
            toTransferDstBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            toTransferDstBarrier.oldLayout = ImageUsageToImageLayout(dstUsage);
            toTransferDstBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            toTransferDstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toTransferDstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toTransferDstBarrier.image = (VkImage)dst->GetNativeHandle();
            toTransferDstBarrier.subresourceRange = distanceRange;

            if (srcUsage != ImageUsage::eTransferSrc)
                barriers[barrierCount++] = toTransferSrcBarrier;
            if (dstUsage != ImageUsage::eTransferDst)
                barriers[barrierCount++] = toTransferDstBarrier;

            if (barrierCount > 0)
            {
                vkCmdPipelineBarrier
                    (
                        mHandle,
                        ImageUsageToPipelineStage(srcUsage) | ImageUsageToPipelineStage(dstUsage),
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        {},
                        0, nullptr,
                        0, nullptr,
                        static_cast<uint32_t>(barrierCount), barriers
                    );
            }

            auto srcLayers = GetImageSubresourceLayers(*src);
            auto dstLayers = GetImageSubresourceLayers(*dst);

            VkImageBlit imageBlitInfo{};
            imageBlitInfo.srcOffsets[0] = VkOffset3D{ 0, 0, 0 };
            imageBlitInfo.srcOffsets[1] = VkOffset3D{ (int32_t)src->GetWidth(), (int32_t)src->GetHeight(), 1 };
            imageBlitInfo.dstOffsets[0] = VkOffset3D{ 0, 0, 0 };
            imageBlitInfo.dstOffsets[1] = VkOffset3D{ (int32_t)dst->GetWidth(), (int32_t)dst->GetHeight(), 1 };
            imageBlitInfo.srcSubresource = srcLayers;
            imageBlitInfo.dstSubresource = dstLayers;

            vkCmdBlitImage
            (
                mHandle,
                (VkImage)src->GetNativeHandle(),
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                (VkImage)dst->GetNativeHandle(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &imageBlitInfo,
                ToVulkanFilter(filter)
            );
        }

        void ImageBarrier(Ref<Image>& image, ImageUsage::Bits src, ImageUsage::Bits dst) const override
        {
            ImageBarrier(*image, src, dst);
        }

        void ImageBarrier(Image& image, ImageUsage::Bits src, ImageUsage::Bits dst) const override
        {
            if (src == dst) return;

            VkImageSubresourceRange imageSubresourceRange = GetImageSubresourceRange(image);

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = ImageUsageToAccessFlags(src);
            barrier.dstAccessMask = ImageUsageToAccessFlags(dst);
            barrier.oldLayout = ImageUsageToImageLayout(src);
            barrier.newLayout = ImageUsageToImageLayout(dst);
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = (VkImage)image.GetNativeHandle();
            barrier.subresourceRange = imageSubresourceRange;

            vkCmdPipelineBarrier
            (
                mHandle,
                ImageUsageToPipelineStage(src),
                ImageUsageToPipelineStage(dst),
                {},
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        }

        void GenerateMipLevels(const Image& image, ImageUsage::Bits initialUsage, Filter filter) const override
        {
            if (image.GetMipLevelsCount() < 2) return;

            auto srcRange = GetImageSubresourceRange(image);
            auto dstRange = GetImageSubresourceRange(image);
            auto srcLayers = GetImageSubresourceLayers(image);
            auto dstLayers = GetImageSubresourceLayers(image);
            auto srcUsage = initialUsage;
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
                
                srcLayers.mipLevel = i;
                srcRange.baseMipLevel = i;
                srcRange.levelCount = 1;

                dstLayers.mipLevel = i + 1;
                dstRange.baseMipLevel = i + 1;
                dstRange.levelCount = 1;

                VkImageMemoryBarrier imageBarrier = {};
                imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // to transfer source
                imageBarrier.srcAccessMask = ImageUsageToAccessFlags(srcUsage);
                imageBarrier.dstAccessMask = ImageUsageToAccessFlags(ImageUsage::eTransferDst);
                imageBarrier.oldLayout = ImageUsageToImageLayout(srcUsage);
                imageBarrier.newLayout = ImageUsageToImageLayout(ImageUsage::eTransferSrc);
                imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageBarrier.image = (VkImage)image.GetNativeHandle();
                imageBarrier.subresourceRange = srcRange;

                vkCmdPipelineBarrier
                    (
                        mHandle,
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        {},
                        0, nullptr,
                        0, nullptr,
                        1, &imageBarrier
                    );

                srcUsage = ImageUsage::eTransferDst;

                VkImageBlit imageBlitInfo{};
                imageBlitInfo.srcOffsets[0] = { 0, 0, 0 };
                imageBlitInfo.srcOffsets[1] = { (int32_t)sourceWidth, (int32_t)sourceHeight, 1 };
                imageBlitInfo.dstOffsets[0] = { 0, 0, 0 };
                imageBlitInfo.dstOffsets[1] = { (int32_t)destinationWidth, (int32_t)destinationWidth, 1 };
                imageBlitInfo.srcSubresource = srcLayers;
                imageBlitInfo.dstSubresource = dstLayers;

                vkCmdBlitImage
                (
                    mHandle,
                    (VkImage)image.GetNativeHandle(),
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    (VkImage)image.GetNativeHandle(),
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &imageBlitInfo,
                    ToVulkanFilter(filter)
                );
            }

            auto mipLevelsSubresourceRange = GetImageSubresourceRange(image);
            mipLevelsSubresourceRange.levelCount = mipLevelsSubresourceRange.levelCount - 1;
            VkImageMemoryBarrier mipLevelsTransfer{};
            mipLevelsTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            mipLevelsTransfer.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            mipLevelsTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            mipLevelsTransfer.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            mipLevelsTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            mipLevelsTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            mipLevelsTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            mipLevelsTransfer.image = (VkImage)image.GetNativeHandle();
            mipLevelsTransfer.subresourceRange = mipLevelsSubresourceRange;

            vkCmdPipelineBarrier
                (
                    mHandle,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    {},
                    0, nullptr,
                    0, nullptr,
                    1, &mipLevelsTransfer
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
