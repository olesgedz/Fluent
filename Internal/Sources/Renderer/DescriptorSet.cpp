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

        void UpdateDescriptorSet(const DescriptorUpdateDesc& updateDesc) override
        {
            vk::DescriptorBufferInfo bufferInfo;
            bufferInfo
                .setBuffer((VkBuffer)updateDesc.bufferUpdate.buffer->GetNativeHandle())
                .setOffset(updateDesc.bufferUpdate.offset)
                .setRange(updateDesc.bufferUpdate.range);

            vk::WriteDescriptorSet writeDescriptorSet;
            writeDescriptorSet
                .setDstBinding(updateDesc.binding)
                .setDstSet(mHandle)
                .setDescriptorType(ToVulkanDescriptorType(updateDesc.descriptorType))
                .setBufferInfo(bufferInfo);

            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            device.updateDescriptorSets(writeDescriptorSet, {});
        }

        Handle GetNativeHandle() const override { return mHandle; }
    };

    Ref<DescriptorSet> DescriptorSet::Create(const DescriptorSetDescription& description)
    {
        return CreateRef<VulkanDescriptorSet>(description);
    }
} // namespace Fluent
