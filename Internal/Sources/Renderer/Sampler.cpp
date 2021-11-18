#include "Renderer/GraphicContext.hpp"
#include "Renderer/Sampler.hpp"

namespace Fluent
{
    class VulkanSampler : public Sampler
    {
    private:
        VkSampler mHandle;
    public:
        VulkanSampler(const SamplerDescription& description)
        {
            VkSamplerCreateInfo samplerCreateInfo{};
            samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerCreateInfo.magFilter = ToVulkanFilter(description.magFilter);
            samplerCreateInfo.minFilter = ToVulkanFilter(description.minFilter);
            samplerCreateInfo.mipmapMode = ToVulkanSamplerMipmapMode(description.mipmapMode);
            samplerCreateInfo.addressModeU = ToVulkanSamplerAddressMode(description.addressModeU);
            samplerCreateInfo.addressModeV = ToVulkanSamplerAddressMode(description.addressModeV);
            samplerCreateInfo.addressModeW = ToVulkanSamplerAddressMode(description.addressModeW);
            samplerCreateInfo.mipLodBias = description.mipLodBias;
            samplerCreateInfo.anisotropyEnable = description.anisotropyEnable;
            samplerCreateInfo.maxAnisotropy = description.maxAnisotropy;
            samplerCreateInfo.compareEnable = description.compareEnable;
            samplerCreateInfo.compareOp = ToVulkanCompareOp(description.compareOp);
            samplerCreateInfo.minLod = description.minLod;
            samplerCreateInfo.maxLod = description.maxLod;

            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            VK_ASSERT(vkCreateSampler(device, &samplerCreateInfo, nullptr, &mHandle));
        }

        ~VulkanSampler() override
        {
            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            vkDestroySampler(device, mHandle, nullptr);
        }

        Handle GetNativeHandle() const override { return mHandle; }
    };

    Ref<Sampler> Sampler::Create(const SamplerDescription& description)
    {
        return CreateRef<VulkanSampler>(description);
    }
} // namespace Fluent
