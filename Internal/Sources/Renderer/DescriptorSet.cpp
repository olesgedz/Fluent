#include "Renderer/GraphicContext.hpp"
#include "Renderer/DescriptorSet.hpp"

namespace Fluent
{
    class VulkanDescriptorSet : public DescriptorSet
    {
    private:
        VkDescriptorSet mHandle;
    public:
        VulkanDescriptorSet(const DescriptorSetDescription& description)
        {
            VkDescriptorPool descriptorPool = (VkDescriptorPool)GetGraphicContext().GetDescriptorPool();
            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            VkDescriptorSetLayout layout = (VkDescriptorSetLayout)description.descriptorSetLayout->GetNativeHandle();

            VkDescriptorSetAllocateInfo descriptorAllocateInfo{};
            descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptorAllocateInfo.descriptorPool = descriptorPool;
            descriptorAllocateInfo.descriptorSetCount = 1;
            descriptorAllocateInfo.pSetLayouts = &layout;

            vkAllocateDescriptorSets(device, &descriptorAllocateInfo, &mHandle);
        }

        ~VulkanDescriptorSet() override = default;

        void UpdateDescriptorSet(const std::vector<DescriptorSetUpdateDesc>& updateDescs) override
        {
            // TODO: Very bad
            std::vector<VkDescriptorBufferInfo> bufferUpdates(updateDescs.size());
            std::vector<VkDescriptorImageInfo> imageUpdates(updateDescs.size());
            std::vector<VkWriteDescriptorSet> descriptorWrites(updateDescs.size());

            uint32_t buffer = 0;
            uint32_t image = 0;
            uint32_t write = 0;

            for (const auto& update : updateDescs)
            {
                auto& writeDescriptorSet = descriptorWrites[write++];
                writeDescriptorSet.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstBinding       = update.binding;
                writeDescriptorSet.descriptorCount  = 1;
                writeDescriptorSet.dstSet           = mHandle;
                writeDescriptorSet.descriptorType   = ToVulkanDescriptorType(update.descriptorType);

                if (update.bufferUpdate.buffer != nullptr)
                {
                    auto& bufferUpdateInfo = bufferUpdates[buffer++];
                    bufferUpdateInfo.buffer = (VkBuffer)update.bufferUpdate.buffer->GetNativeHandle();
                    bufferUpdateInfo.offset = update.bufferUpdate.offset;
                    bufferUpdateInfo.range = update.bufferUpdate.range;

                    writeDescriptorSet.pBufferInfo = &bufferUpdateInfo;
                }

                if (update.imageUpdate.sampler != nullptr || update.imageUpdate.image != nullptr)
                {
                    auto& imageUpdateInfo = imageUpdates[image++];
                    if (update.imageUpdate.sampler != nullptr)
                    {
                        imageUpdateInfo.sampler = (VkSampler)update.imageUpdate.sampler->GetNativeHandle();
                    }

                    if (update.imageUpdate.image != nullptr)
                    {
                        imageUpdateInfo.imageLayout = ImageUsageToImageLayout(update.imageUpdate.usage);
                        imageUpdateInfo.imageView = (VkImageView)update.imageUpdate.image->GetImageView();
                    }

                    writeDescriptorSet.pImageInfo = &imageUpdateInfo;
                }
            }

            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            vkUpdateDescriptorSets(device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
        }

        Handle GetNativeHandle() const override { return mHandle; }
    };

    Ref<DescriptorSet> DescriptorSet::Create(const DescriptorSetDescription& description)
    {
        return CreateRef<VulkanDescriptorSet>(description);
    }
} // namespace Fluent
