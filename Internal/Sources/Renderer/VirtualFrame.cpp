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
            vk::Semaphore       renderCompleteSemaphore;
            Ref<CommandBuffer>  cmd;
            vk::Fence fence;
        };
    private:
        vk::Device                  mDevice;
        vk::CommandPool             mCommandPool;
        vk::SwapchainKHR            mSwapchain;
        vk::Queue                   mQueue;
        uint32_t                    mCurrentFrameIndex = 0;
        std::vector<vk::Fence>      mImagesInFlight;
        vk::Semaphore               mAcquireSemaphore;
        std::vector<VirtualFrame>   mVirtualFrames;
        uint32_t                    mActiveImageIndex;
    public:
        VulkanFrameProvider(const VirtualFrameProviderDescription& description)
            : mDevice((VkDevice)description.device)
            , mCommandPool((VkCommandPool)description.commandPool)
            , mSwapchain((VkSwapchainKHR)description.swapchain)
            , mQueue((VkQueue)description.queue)
            , mImagesInFlight(description.swapchainImageCount, nullptr)
        {
            mAcquireSemaphore = mDevice.createSemaphore(vk::SemaphoreCreateInfo{});

            CommandBufferDescription cmdDesc{};
            cmdDesc.commandPool = description.commandPool;
            cmdDesc.device = description.device;

            mVirtualFrames.resize(description.frameCount);

            StagingBufferDescription stagingBufferDesc{};
            stagingBufferDesc.size = description.stagingBufferSize;

            for (uint32_t i = 0; i < description.frameCount; ++i)
            {
                mVirtualFrames[i].renderCompleteSemaphore = mDevice.createSemaphore(vk::SemaphoreCreateInfo{});
                mVirtualFrames[i].cmd = CommandBuffer::Create(cmdDesc);
                mVirtualFrames[i].fence = mDevice.createFence(vk::FenceCreateInfo { vk::FenceCreateFlagBits::eSignaled });
                mVirtualFrames[i].stagingBuffer = StagingBuffer::Create(stagingBufferDesc);
            }
        }

        ~VulkanFrameProvider() override
        {
            for (auto& frame : mVirtualFrames)
                mDevice.destroyFence(frame.fence);
        }

        bool BeginFrame() override
        {
            try
            {
                auto acquireResult = mDevice.acquireNextImageKHR
                (
                    mSwapchain,
                    std::numeric_limits<uint64_t>::max(),
                    mAcquireSemaphore
                );

                mActiveImageIndex = acquireResult.value;
            }
            catch(vk::OutOfDateKHRError&)
            {
                return false;
            }

            auto waitResult = mDevice.waitForFences(mVirtualFrames[mCurrentFrameIndex].fence, true, std::numeric_limits<uint64_t>::max());

            if (mImagesInFlight[mActiveImageIndex])
            {
                waitResult = mDevice.waitForFences(mImagesInFlight[mActiveImageIndex], true, std::numeric_limits<uint64_t>::max());
            }

            mImagesInFlight[mActiveImageIndex] = mVirtualFrames[mCurrentFrameIndex].fence;

            mDevice.resetFences(mVirtualFrames[mCurrentFrameIndex].fence);

            /// Recording command buffers
            auto& cmd = mVirtualFrames[mCurrentFrameIndex].cmd;
            cmd->Begin();
            return true;
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
                .setWaitSemaphores(mAcquireSemaphore)
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

            try
            {
                auto presentResult = mQueue.presentKHR(presentInfo);
            }
            catch(vk::OutOfDateKHRError&)
            {
                return false;
            }

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
