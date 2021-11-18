#include "Renderer/GraphicContext.hpp"
#include "Renderer/Framebuffer.hpp"

namespace Fluent
{
    class VulkanFramebuffer : public Framebuffer
    {
    private:
        VkFramebuffer mHandle = nullptr;
    public:
        VulkanFramebuffer(const FramebufferDescription& description)
        {
            std::vector<VkImageView> attachmentViews;

            for (auto& target : description.targets)
                attachmentViews.emplace_back((VkImageView)target->GetImageView());
            if (description.depthStencil)
                attachmentViews.emplace_back((VkImageView)description.depthStencil->GetImageView());
                
            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = (VkRenderPass)description.renderPass->GetNativeHandle();
            framebufferCreateInfo.attachmentCount = attachmentViews.size();
            framebufferCreateInfo.pAttachments = attachmentViews.data();
            framebufferCreateInfo.width = description.width;
            framebufferCreateInfo.height = description.height;
            framebufferCreateInfo.layers = 1;

            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &mHandle);
        }

        ~VulkanFramebuffer() override
        {
            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            vkDestroyFramebuffer(device, mHandle, nullptr);
        }

        Handle GetNativeHandle() const override { return mHandle; }
    };

    Ref<Framebuffer> Framebuffer::Create(const FramebufferDescription& description)
    {
        return CreateRef<VulkanFramebuffer>(description);
    }
} // namespace Fluent
