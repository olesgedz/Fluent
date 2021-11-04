#include "Renderer/GraphicContext.hpp"
#include "Renderer/DescriptorSet.hpp"

namespace Fluent
{
    class VulkanDescriptorSet : public DescriptorSet
    {
    private:
        vk::DescriptorSet mHandle;
    public:
        VulkanDescriptorSet(const DescriptorSetDescription& description)
        {
            vk::DescriptorPool descriptorPool = (VkDescriptorPool)GetGraphicContext().GetDescriptorPool();
            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            vk::DescriptorSetLayout layout = (VkDescriptorSetLayout)description.descriptorSetLayout->GetNativeHandle();

            vk::DescriptorSetAllocateInfo descriptorAllocateInfo;
            descriptorAllocateInfo
                .setDescriptorPool(descriptorPool)
                .setSetLayouts(layout);

            mHandle = device.allocateDescriptorSets(descriptorAllocateInfo).front();
        }

        ~VulkanDescriptorSet() override
        {
        }

        void UpdateDescriptorSet(const std::span<DescriptorSetUpdateDesc>& updateDescs) override
        {
            // TODO: Very bad
            std::vector<vk::DescriptorBufferInfo> bufferUpdates;
            std::vector<vk::DescriptorImageInfo> imageUpdates;
            std::vector<vk::WriteDescriptorSet> descriptorWrites;

            for (const auto& update : updateDescs)
            {
                auto& writeDescriptorSet = descriptorWrites.emplace_back();
                writeDescriptorSet
                    .setDstBinding(update.binding)
                    .setDstSet(mHandle)
                    .setDescriptorType(ToVulkanDescriptorType(update.descriptorType));

                if (update.bufferUpdate.buffer != nullptr)
                {
                    auto& bufferUpdateInfo = bufferUpdates.emplace_back();
                    bufferUpdateInfo
                        .setBuffer((VkBuffer)update.bufferUpdate.buffer->GetNativeHandle())
                        .setOffset(update.bufferUpdate.offset)
                        .setRange(update.bufferUpdate.range);
                        
                    writeDescriptorSet.setBufferInfo(bufferUpdateInfo);
                }

                if (update.imageUpdate.image != nullptr)
                {
                    auto& imageUpdateInfo = imageUpdates.emplace_back();
                    imageUpdateInfo
                        .setImageLayout(ImageUsageToImageLayout(update.imageUpdate.usage))
                        .setImageView((VkImageView)update.imageUpdate.image->GetImageView());
                    
                    writeDescriptorSet.setImageInfo(imageUpdateInfo);
                }

                if (update.imageUpdate.sampler != nullptr || update.imageUpdate.image != nullptr)
                {
                    auto& imageUpdateInfo = imageUpdates.emplace_back();

                    if (update.imageUpdate.sampler != nullptr)
                    {
                        imageUpdateInfo.setSampler((VkSampler)update.imageUpdate.sampler->GetNativeHandle());
                    }

                    if (update.imageUpdate.image != nullptr)
                    {
                        imageUpdateInfo
                            .setImageLayout(ImageUsageToImageLayout(update.imageUpdate.usage))
                            .setImageView((VkImageView)update.imageUpdate.image->GetImageView());
                    }

                    writeDescriptorSet.setImageInfo(imageUpdateInfo);
                }
            }

            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            device.updateDescriptorSets(descriptorWrites, {});
        }

        Handle GetNativeHandle() const override { return mHandle; }
    };

    Ref<DescriptorSet> DescriptorSet::Create(const DescriptorSetDescription& description)
    {
        return CreateRef<VulkanDescriptorSet>(description);
    }
} // namespace Fluent
