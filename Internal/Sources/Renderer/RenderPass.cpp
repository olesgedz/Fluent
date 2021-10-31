#include <array>
#include "Renderer/GraphicContext.hpp"
#include "Renderer/RenderPass.hpp"
#include "Renderer/Image.hpp"

namespace Fluent
{
    class VulkanPass : public RenderPass
    {
    private:
        uint32_t mWidth;
        uint32_t mHeight;
        vk::RenderPass mHandle;
        std::vector<ClearValue> mClearValues;
        float mDepth;
        uint32_t mStencil;
        bool mHasDepthStencil = false;
    public:
        explicit VulkanPass(const RenderPassDescription& description)
            : mWidth(description.width)
            , mHeight(description.height)
            , mHasDepthStencil(false)
        {
            uint32_t attachmentsCount = description.attachmentLoadOps.size();
            std::vector<vk::AttachmentDescription> attachmentDescriptions;
            std::vector<vk::AttachmentReference> attachmentReferences;

            vk::AttachmentReference depthStencilAttachmentReference;

            for (uint32_t i = 0; i < attachmentsCount; ++i)
            {
                vk::AttachmentDescription attachmentDescription;
                attachmentDescription
                    .setFormat(ToVulkanFormat(description.colorFormats[i]))
                    .setSamples(ToVulkanSampleCount(description.sampleCount))
                    .setLoadOp(ToVulkanLoadOp(description.attachmentLoadOps[i]))
                    .setStoreOp(vk::AttachmentStoreOp::eStore)
                    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                    .setInitialLayout(ImageUsageToImageLayout(description.initialUsages[i]))
                    .setFinalLayout(ImageUsageToImageLayout(description.finalUsages[i]));

                vk::AttachmentReference attachmentReference;
                attachmentReference
                    .setAttachment(i)
                    .setLayout(vk::ImageLayout::eColorAttachmentOptimal); // TODO
                
                if (description.finalUsages[i] == ImageUsage::eDepthStencilAttachment)
                {
                    depthStencilAttachmentReference = attachmentReference;
                    attachmentDescription
                        .setLoadOp(vk::AttachmentLoadOp::eDontCare)
                        .setStencilLoadOp(ToVulkanLoadOp(description.depthLoadOp))
                        .setStencilStoreOp(vk::AttachmentStoreOp::eStore);

                    mHasDepthStencil = true;
                    mDepth = description.clearValues[i].depth;
                    mStencil = description.clearValues[i].stencil;
                }
                else
                {
                    attachmentReferences.push_back(attachmentReference);
                }

                mClearValues = description.clearValues;
                attachmentDescriptions.push_back(std::move(attachmentDescription));
            }

            vk::SubpassDescription subpassDescription;
            subpassDescription
                .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachments(attachmentReferences)
                .setPDepthStencilAttachment(depthStencilAttachmentReference != vk::AttachmentReference{ } ?
                    std::addressof(depthStencilAttachmentReference) : nullptr
                );

            // TODO
            std::array subpassDependencies = {
                vk::SubpassDependency {
                    VK_SUBPASS_EXTERNAL,
                    0,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
                    vk::AccessFlagBits::eMemoryRead,
                    vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                    vk::DependencyFlagBits::eByRegion
                },
                vk::SubpassDependency {
                    0,
                    VK_SUBPASS_EXTERNAL,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
                    vk::PipelineStageFlagBits::eBottomOfPipe,
                    vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                    vk::AccessFlagBits::eMemoryRead,
                    vk::DependencyFlagBits::eByRegion
                },
            };

            vk::RenderPassCreateInfo renderPassCreateInfo;
            renderPassCreateInfo
                .setAttachments(attachmentDescriptions)
                .setSubpasses(subpassDescription)
                .setDependencies(subpassDependencies);
            
            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();

            mHandle = device.createRenderPass(renderPassCreateInfo);
        }

        ~VulkanPass() override
        {
            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            device.destroyRenderPass(mHandle);
        }

        const std::vector<ClearValue>& GetClearValues() const override { return mClearValues; }

        Handle GetNativeHandle() const
        {
            return mHandle;
        }

        void SetRenderArea(uint32_t width, uint32_t height) override
        {
            mWidth = width;
            mHeight = height;
        }

        bool HasDepthStencil() const override { return mHasDepthStencil; }
        float GetDepth() const override { return mDepth; }
        uint32_t GetStencil() const override { return mStencil; }
        uint32_t GetWidth() const override { return mWidth; }
        uint32_t GetHeight() const override { return mHeight; }
    };

    Ref<RenderPass> RenderPass::Create(const RenderPassDescription& description)
    {
        return CreateRef<VulkanPass>(description);
    }
}