#include <vulkan/vulkan.hpp>
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
            vk::Semaphore       acquireSemaphore;
            vk::Semaphore       renderCompleteSemaphore;
            Ref<CommandBuffer>  cmd;
            vk::Fence           fence;
        };
    private:
        vk::Device                  mDevice;
        vk::CommandPool             mCommandPool;
        vk::SwapchainKHR            mSwapchain;
        vk::Queue                   mQueue;
        uint32_t                    mCurrentFrameIndex = 0;
        std::vector<bool>           mCommandBuffersRecorded;
        std::vector<VirtualFrame>   mVirtualFrames;
        uint32_t                    mActiveImageIndex;
    public:
        VulkanFrameProvider(const VirtualFrameProviderDescription& description)
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
                mVirtualFrames[i].renderCompleteSemaphore = mDevice.createSemaphore(vk::SemaphoreCreateInfo{});
                mVirtualFrames[i].acquireSemaphore = mDevice.createSemaphore(vk::SemaphoreCreateInfo{});
                mVirtualFrames[i].cmd = CommandBuffer::Create(cmdDesc);
                mVirtualFrames[i].fence = mDevice.createFence(vk::FenceCreateInfo { vk::FenceCreateFlagBits::eSignaled });
                mVirtualFrames[i].stagingBuffer = StagingBuffer::Create(stagingBufferDesc);
            }
        }

        ~VulkanFrameProvider() override
        {
            for (auto& frame : mVirtualFrames)
            {
                frame.stagingBuffer = nullptr;
                mDevice.destroyFence(frame.fence);
                mDevice.destroySemaphore(frame.renderCompleteSemaphore);
                mDevice.destroy(frame.acquireSemaphore);
            }
        }

        bool BeginFrame() override
        {
            bool result = true;

            auto acquireResult = mDevice.acquireNextImageKHR
            (
                mSwapchain,
                std::numeric_limits<uint64_t>::max(),
                mVirtualFrames[mCurrentFrameIndex].acquireSemaphore
            );

            mActiveImageIndex = acquireResult.value;

            if (acquireResult.result != vk::Result::eSuccess) result = false;

            if (mCommandBuffersRecorded[mCurrentFrameIndex] == false)
            {
                auto waitResult = mDevice.waitForFences(mVirtualFrames[mCurrentFrameIndex].fence, true, std::numeric_limits<uint64_t>::max());
                mDevice.resetFences(mVirtualFrames[mCurrentFrameIndex].fence);
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

            vk::ImageSubresourceRange imageSubresourceRange = GetImageSubresourceRange(*image);

            vk::ImageMemoryBarrier transferDstToPresent;
            transferDstToPresent
                .setSrcAccessMask(ImageUsageToAccessFlags(imageUsage))
                .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
                .setOldLayout(ImageUsageToImageLayout(imageUsage))
                .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setImage((VkImage)image->GetNativeHandle())
                .setSubresourceRange(imageSubresourceRange);

            vk::CommandBuffer nativeCmd = vk::CommandBuffer((VkCommandBuffer)cmd->GetNativeHandle());

            nativeCmd.pipelineBarrier
            (
                ImageUsageToPipelineStage(imageUsage),
                vk::PipelineStageFlagBits::eBottomOfPipe,
                {}, {}, {},
                transferDstToPresent
            );

            cmd->End();

            std::array waitDstStageMask = { static_cast<vk::PipelineStageFlags>(vk::PipelineStageFlagBits::eTransfer) };

            vk::SubmitInfo submitInfo;
            submitInfo
                .setWaitSemaphores(mVirtualFrames[mCurrentFrameIndex].acquireSemaphore)
                .setWaitDstStageMask(waitDstStageMask)
                .setSignalSemaphores(mVirtualFrames[mCurrentFrameIndex].renderCompleteSemaphore)
                .setCommandBuffers(nativeCmd);

            mQueue.submit(submitInfo, mVirtualFrames[mCurrentFrameIndex].fence);
    
            vk::PresentInfoKHR presentInfo;
            presentInfo
                .setWaitSemaphores(mVirtualFrames[mCurrentFrameIndex].renderCompleteSemaphore)
                .setSwapchains(mSwapchain)
                .setImageIndices(mActiveImageIndex);
            
            mVirtualFrames[mCurrentFrameIndex].stagingBuffer->Flush();
            mVirtualFrames[mCurrentFrameIndex].stagingBuffer->Reset();

            auto presentResult = mQueue.presentKHR(presentInfo);
            if (presentResult != vk::Result::eSuccess) return false;

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
