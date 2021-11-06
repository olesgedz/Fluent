#include "Renderer/GraphicContext.hpp"
#include "Renderer/Sampler.hpp"

namespace Fluent
{
    class VulkanSampler : public Sampler
    {
    private:
        vk::Sampler mHandle;
    public:
        VulkanSampler(const SamplerDescription& description)
        {
            vk::SamplerCreateInfo samplerCreateInfo;
            samplerCreateInfo
                .setMagFilter(ToVulkanFilter(description.magFilter))
                .setMinFilter(ToVulkanFilter(description.minFilter))
                .setMipmapMode(ToVulkanSamplerMipmapMode(description.mipmapMode))
                .setAddressModeU(ToVulkanSamplerAddressMode(description.addressModeU))
                .setAddressModeV(ToVulkanSamplerAddressMode(description.addressModeV))
                .setAddressModeW(ToVulkanSamplerAddressMode(description.addressModeW))
                .setMipLodBias(description.mipLodBias)
                .setAnisotropyEnable(description.anisotropyEnable)
                .setMaxAnisotropy(description.maxAnisotropy)
                .setCompareEnable(description.compareEnable)
                .setCompareOp(ToVulkanCompareOp(description.compareOp))
                .setMinLod(description.minLod)
                .setMaxLod(description.maxLod);
            
            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            mHandle = device.createSampler(samplerCreateInfo);
        }

        ~VulkanSampler() override
        {
            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            device.destroySampler(mHandle);
        }

        Handle GetNativeHandle() const override { return mHandle; }
    };

    Ref<Sampler> Sampler::Create(const SamplerDescription& description)
    {
        return CreateRef<VulkanSampler>(description);
    }
} // namespace Fluent
