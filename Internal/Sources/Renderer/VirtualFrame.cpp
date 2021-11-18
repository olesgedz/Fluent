#include "Renderer/Renderer.hpp"
#include "Renderer/GraphicContext.hpp"
#include "Renderer/VirtualFrame.hpp"
#include "Renderer/StagingBuffer.hpp"

namespace Fluent
{
    class VulkanFrameProvider : public VirtualFrameProvider
    {
        struct VirtualFrame
        {
            Ref<StagingBuffer>  stagingBuffer;
            VkSemaphore         acquireSemaphore;
            VkSemaphore         renderCompleteSemaphore;
            Ref<CommandBuffer>  cmd;
            VkFence             fence;
        };
    private:
        VkDevice                    mDevice;
        VkCommandPool               mCommandPool;
        VkSwapchainKHR              mSwapchain;
        VkQueue                     mQueue;
        uint32_t                    mCurrentFrameIndex = 0;
        std::vector<bool>           mCommandBuffersRecorded;
        std::vector<VirtualFrame>   mVirtualFrames;
        uint32_t                    mActiveImageIndex{};
    public:
        explicit VulkanFrameProvider(const VirtualFrameProviderDescription& description)
            : mDevice((VkDevice)description.device)
            , mCommandPool((VkCommandPool)description.commandPool)
            , mSwapchain((VkSwapchainKHR)description.swapchain)
            , mQueue((VkQueue)description.queue)
            , mCurrentFrameIndex(0)
        {
            mCommandBuffersRecorded.resize(description.frameCount);
            std::fill(mCommandBuffersRecorded.begin(), mCommandBuffersRecorded.end(), false);

            CommandBufferDescription cmdDesc{};
            cmdDesc.commandPool = description.commandPool;
            cmdDesc.device = description.device;

            mVirtualFrames.resize(description.frameCount);

            StagingBufferDescription stagingBufferDesc{};
            stagingBufferDesc.size = description.stagingBufferSize;

            for (uint32_t i = 0; i < description.frameCount; ++i)
            {
                VkSemaphoreCreateInfo semaphoreCreateInfo{};
                semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mVirtualFrames[i].renderCompleteSemaphore);
                vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mVirtualFrames[i].acquireSemaphore);

                mVirtualFrames[i].cmd = CommandBuffer::Create(cmdDesc);

                VkFenceCreateInfo fenceCreateInfo{};
                fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
                vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mVirtualFrames[i].fence);

                mVirtualFrames[i].stagingBuffer = StagingBuffer::Create(stagingBufferDesc);
            }
        }

        ~VulkanFrameProvider() override
        {
            for (auto& frame : mVirtualFrames)
            {
                frame.stagingBuffer = nullptr;
                vkDestroyFence(mDevice, frame.fence, nullptr);
                vkDestroySemaphore(mDevice, frame.renderCompleteSemaphore, nullptr);
                vkDestroySemaphore(mDevice, frame.acquireSemaphore, nullptr);
            }
        }

        bool BeginFrame() override
        {
            bool result = true;

            auto acquireResult = vkAcquireNextImageKHR
                (
                    mDevice, mSwapchain,
                    std::numeric_limits<uint64_t>::max(),
                    mVirtualFrames[mCurrentFrameIndex].acquireSemaphore,
                    VK_NULL_HANDLE,
                    &mActiveImageIndex
                );

            if (acquireResult != VK_SUCCESS) result = false;

            if (!mCommandBuffersRecorded[mCurrentFrameIndex])
            {
                vkWaitForFences(mDevice, 1, &mVirtualFrames[mCurrentFrameIndex].fence, true, std::numeric_limits<uint64_t>::max());
                vkResetFences(mDevice, 1, &mVirtualFrames[mCurrentFrameIndex].fence);
                mCommandBuffersRecorded[mCurrentFrameIndex] = true;
            }

            /// Recording command buffers
            auto& cmd = mVirtualFrames[mCurrentFrameIndex].cmd;
            cmd->Begin();
            return result;
        }

        bool EndFrame() override
        {
            auto& cmd = mVirtualFrames[mCurrentFrameIndex].cmd;

            auto imageUsage = GetGraphicContext().GetSwapchainImageUsage(mActiveImageIndex);
            auto image = GetGraphicContext().AcquireImage(mActiveImageIndex, ImageUsage::eUndefined);

            VkImageSubresourceRange imageSubresourceRange = GetImageSubresourceRange(*image);

            VkImageMemoryBarrier transferDstToPresent{};
            transferDstToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            transferDstToPresent.srcAccessMask = ImageUsageToAccessFlags(imageUsage);
            transferDstToPresent.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            transferDstToPresent.oldLayout = ImageUsageToImageLayout(imageUsage);
            transferDstToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            transferDstToPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            transferDstToPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            transferDstToPresent.image = (VkImage)image->GetNativeHandle();
            transferDstToPresent.subresourceRange = imageSubresourceRange;

            auto nativeCmd = (VkCommandBuffer)cmd->GetNativeHandle();

            vkCmdPipelineBarrier
            (
                nativeCmd,
                ImageUsageToPipelineStage(imageUsage),
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &transferDstToPresent
            );

            cmd->End();

            VkPipelineStageFlags waitDstStageMask[] = { VK_PIPELINE_STAGE_TRANSFER_BIT };

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &mVirtualFrames[mCurrentFrameIndex].acquireSemaphore;
            submitInfo.pWaitDstStageMask = waitDstStageMask;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &mVirtualFrames[mCurrentFrameIndex].renderCompleteSemaphore;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &nativeCmd;

            vkQueueSubmit(mQueue, 1, &submitInfo, mVirtualFrames[mCurrentFrameIndex].fence);

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &mVirtualFrames[mCurrentFrameIndex].renderCompleteSemaphore;
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &mSwapchain;
            presentInfo.pImageIndices = &mActiveImageIndex;

            mVirtualFrames[mCurrentFrameIndex].stagingBuffer->Flush();
            mVirtualFrames[mCurrentFrameIndex].stagingBuffer->Reset();

            auto presentResult = vkQueuePresentKHR(mQueue, &presentInfo);

            if (presentResult != VK_SUCCESS) return false;

            mCommandBuffersRecorded[mCurrentFrameIndex] = false;
            mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mVirtualFrames.size();

            return true;
        }

        Ref<StagingBuffer>& GetStagingBuffer() override
        {
            return mVirtualFrames[mCurrentFrameIndex].stagingBuffer;
        }

        uint32_t GetActiveImageIndex() const override { return mActiveImageIndex; }
        Ref<CommandBuffer>& GetCommandBuffer() override { return mVirtualFrames[mCurrentFrameIndex].cmd; };
    };
    
    /// Interface

    Scope<VirtualFrameProvider> VirtualFrameProvider::Create(const VirtualFrameProviderDescription& description)
    {
        return CreateScope<VulkanFrameProvider>(description);
    };
} // namespace Fluent
