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
            std::vector<std::vector<VkDescriptorBufferInfo>> bufferUpdates(updateDescs.size());
            std::vector<std::vector<VkDescriptorImageInfo>> imageUpdates(updateDescs.size());
            std::vector<VkWriteDescriptorSet> descriptorWrites(updateDescs.size());

            uint32_t bufferUpdateIdx = 0;
            uint32_t imageUpdateIdx = 0;
            uint32_t write = 0;

            for (const auto& update : updateDescs)
            {
                auto& writeDescriptorSet = descriptorWrites[write++];
                writeDescriptorSet.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstBinding       = update.binding;
                writeDescriptorSet.descriptorCount  = 1;
                writeDescriptorSet.dstSet           = mHandle;
                writeDescriptorSet.descriptorType   = ToVulkanDescriptorType(update.descriptorType);

                if (!update.bufferUpdates.empty())
                {
                    auto& bufferUpdateInfos = bufferUpdates[bufferUpdateIdx++];
                    bufferUpdateInfos.resize(update.bufferUpdates.size(), {});

                    for (uint32_t i = 0; i < bufferUpdateInfos.size(); ++i)
                    {
                        bufferUpdateInfos[i].buffer = (VkBuffer)update.bufferUpdates[i].buffer->GetNativeHandle();
                        bufferUpdateInfos[i].offset = update.bufferUpdates[i].offset;
                        bufferUpdateInfos[i].range = update.bufferUpdates[i].range;
                    }

                    writeDescriptorSet.descriptorCount = bufferUpdateInfos.size();
                    writeDescriptorSet.pBufferInfo = bufferUpdateInfos.data();
                }

                if (!update.imageUpdates.empty())
                {
                    auto& imageUpdateInfos = imageUpdates[imageUpdateIdx++];
                    imageUpdateInfos.resize(update.imageUpdates.size(), {});

                    for (uint32_t i = 0; i < imageUpdateInfos.size(); ++i)
                    {
                        if (update.imageUpdates[i].sampler != nullptr)
                        {
                            imageUpdateInfos[i].sampler = (VkSampler)update.imageUpdates[i].sampler->GetNativeHandle();
                        }

                        if (update.imageUpdates[i].image != nullptr)
                        {
                            imageUpdateInfos[i].imageLayout = ImageUsageToImageLayout(update.imageUpdates[i].usage);
                            imageUpdateInfos[i].imageView = (VkImageView)update.imageUpdates[i].image->GetImageView();
                        }
                    }

                    writeDescriptorSet.descriptorCount = imageUpdateInfos.size();
                    writeDescriptorSet.pImageInfo = imageUpdateInfos.data();
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
