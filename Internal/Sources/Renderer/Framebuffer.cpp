#include "Renderer/GraphicContext.hpp"
#include "Renderer/Framebuffer.hpp"

namespace Fluent
{
    class VulkanFramebuffer : public Framebuffer
    {
    private:
        vk::Framebuffer mHandle;
    public:
        VulkanFramebuffer(const FramebufferDescription& description)
        {
            std::vector<vk::ImageView> attachmentViews;

            for (auto& target : description.targets)
                attachmentViews.emplace_back((VkImageView)target->GetImageView());
            if (description.depthStencil)
                attachmentViews.emplace_back((VkImageView)description.depthStencil->GetImageView());
                
            vk::FramebufferCreateInfo framebufferCreateInfo;
            framebufferCreateInfo
                .setRenderPass((VkRenderPass)description.renderPass->GetNativeHandle())
                .setAttachments(attachmentViews)
                .setWidth(description.renderPass->GetWidth())
                .setHeight(description.renderPass->GetHeight())
                .setLayers(1);

            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            mHandle = device.createFramebuffer(framebufferCreateInfo);
        }

        ~VulkanFramebuffer() override
        {
            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            device.destroyFramebuffer(mHandle);
        }

        Handle GetNativeHandle() const override { return mHandle; }
    };

    Ref<Framebuffer> Framebuffer::Create(const FramebufferDescription& description)
    {
        return CreateRef<VulkanFramebuffer>(description);
    }
} // namespace Fluent
