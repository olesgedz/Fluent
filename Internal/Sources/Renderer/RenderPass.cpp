#include <array>
#include <optional>
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
        VkRenderPass mHandle;
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
            uint32_t attachmentsCount = description.initialUsages.size();
            std::vector<VkAttachmentDescription> attachmentDescriptions;
            std::vector<VkAttachmentReference> attachmentReferences;

            std::optional<VkAttachmentReference> depthStencilAttachmentReference;

            mClearValues.reserve(description.clearValues.size());

            for (uint32_t i = 0; i < attachmentsCount; ++i)
            {
                VkAttachmentDescription attachmentDescription{};
                attachmentDescription.format = ToVulkanFormat(description.colorFormats[i]);
                attachmentDescription.samples = ToVulkanSampleCount(description.sampleCount);
                attachmentDescription.loadOp = ToVulkanLoadOp(description.attachmentLoadOps[i]);
                attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachmentDescription.initialLayout = ImageUsageToImageLayout(description.initialUsages[i]);
                attachmentDescription.finalLayout = ImageUsageToImageLayout(description.finalUsages[i]);

                VkAttachmentReference attachmentReference{};
                attachmentReference.attachment = i;
                attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                if (description.finalUsages[i] == ImageUsage::eDepthStencilAttachment)
                {
                    attachmentDescription.format = ToVulkanFormat(description.depthStencilFormat);
                    attachmentDescription.loadOp = ToVulkanLoadOp(description.depthLoadOp);
                    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachmentDescription.stencilLoadOp = ToVulkanLoadOp(description.depthLoadOp);
                    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

                    attachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                    depthStencilAttachmentReference = attachmentReference;

                    mHasDepthStencil = true;
                    mDepth = description.clearValues[i].depth;
                    mStencil = description.clearValues[i].stencil;
                }
                else
                {
                    attachmentReferences.emplace_back(attachmentReference);
                    mClearValues.emplace_back(description.clearValues[i]);
                }

                attachmentDescriptions.emplace_back(attachmentDescription);
            }

            VkSubpassDescription subpassDescription{};
            subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription.colorAttachmentCount = attachmentReferences.size();
            subpassDescription.pColorAttachments = attachmentReferences.data();
            subpassDescription.pDepthStencilAttachment = depthStencilAttachmentReference.has_value() ?
                                                        &depthStencilAttachmentReference.value() : nullptr;

            // TODO
            std::array subpassDependencies = {
                VkSubpassDependency {
                    VK_SUBPASS_EXTERNAL,
                    0,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                    VK_ACCESS_MEMORY_READ_BIT,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    VK_DEPENDENCY_BY_REGION_BIT
                },
                VkSubpassDependency {
                    0,
                    VK_SUBPASS_EXTERNAL,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    VK_ACCESS_MEMORY_READ_BIT,
                    VK_DEPENDENCY_BY_REGION_BIT
                },
            };

            VkRenderPassCreateInfo renderPassCreateInfo{};
            renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
            renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
            renderPassCreateInfo.subpassCount = 1;
            renderPassCreateInfo.pSubpasses = &subpassDescription;
            renderPassCreateInfo.dependencyCount = subpassDependencies.size();
            renderPassCreateInfo.pDependencies = subpassDependencies.data();

            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            VK_ASSERT(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &mHandle));
        }

        ~VulkanPass() override
        {
            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            vkDestroyRenderPass(device, mHandle, nullptr);
        }

        const std::vector<ClearValue>& GetClearValues() const override { return mClearValues; }

        Handle GetNativeHandle() const override
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