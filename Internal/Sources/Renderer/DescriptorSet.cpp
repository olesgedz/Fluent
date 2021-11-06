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

        void UpdateDescriptorSet(const std::vector<DescriptorSetUpdateDesc>& updateDescs) override
        {
            // TODO: Very bad
            std::vector<vk::DescriptorBufferInfo> bufferUpdates(updateDescs.size());
            std::vector<vk::DescriptorImageInfo> imageUpdates(updateDescs.size());
            std::vector<vk::WriteDescriptorSet> descriptorWrites(updateDescs.size());

            uint32_t buffer = 0;
            uint32_t image = 0;
            uint32_t write = 0;

            for (const auto& update : updateDescs)
            {
                auto& writeDescriptorSet = descriptorWrites[write++];
                writeDescriptorSet
                    .setDstBinding(update.binding)
                    .setDstSet(mHandle)
                    .setDescriptorType(ToVulkanDescriptorType(update.descriptorType));

                if (update.bufferUpdate.buffer != nullptr)
                {
                    auto& bufferUpdateInfo = bufferUpdates[buffer++];
                    bufferUpdateInfo
                        .setBuffer((VkBuffer)update.bufferUpdate.buffer->GetNativeHandle())
                        .setOffset(update.bufferUpdate.offset)
                        .setRange(update.bufferUpdate.range);
                        
                    writeDescriptorSet.setBufferInfo(bufferUpdateInfo);
                }

                if (update.imageUpdate.sampler != nullptr || update.imageUpdate.image != nullptr)
                {
                    auto& imageUpdateInfo = imageUpdates[image++];
                    if (update.imageUpdate.sampler != nullptr)
                    {
                        imageUpdateInfo
                            .setSampler((VkSampler)update.imageUpdate.sampler->GetNativeHandle());
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
