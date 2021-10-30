#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "Renderer/DeviceAllocator.hpp"

namespace Fluent
{
    class VulkanAllocator : public DeviceAllocator
    {
    private:
        vk::Instance        mInstance;
        vk::PhysicalDevice  mPhysicalDevice;
        vk::Device          mDevice;
        VmaAllocator        mAllocator;
    public:
        VulkanAllocator(const DeviceAllocatorDescription& description)
            : mInstance(static_cast<VkInstance>(description.instance))
            , mPhysicalDevice(static_cast<VkPhysicalDevice>(description.physicalDevice))
            , mDevice(static_cast<VkDevice>(description.device))
        {
            VmaAllocatorCreateInfo allocatorCreateInfo{};
            allocatorCreateInfo.vulkanApiVersion    = VK_API_VERSION_1_2;
            allocatorCreateInfo.instance            = mInstance;
            allocatorCreateInfo.physicalDevice      = mPhysicalDevice;
            allocatorCreateInfo.device              = mDevice;
            vmaCreateAllocator(&allocatorCreateInfo, &mAllocator);
        }

        ~VulkanAllocator() override
        {
            vmaDestroyAllocator(mAllocator);
        }

        AllocatedImage AllocateImage(const ImageDescription& description, MemoryUsage memoryUsage) override
        {
            VmaAllocation allocation;
            VkImage image;

            VmaAllocationCreateInfo allocationCreateInfo{};
            allocationCreateInfo.usage = static_cast<VmaMemoryUsage>(memoryUsage);

            vk::ImageCreateInfo imageCreateInfo;
            imageCreateInfo
                .setArrayLayers(description.arraySize)
                .setFormat(ToVulkanFormat(description.format))
                .setExtent({ description.width, description.height, description.depth })
                .setMipLevels(description.mipLevels)
                .setSamples(ToVulkanSampleCount(description.sampleCount))
                .setTiling(vk::ImageTiling::eOptimal)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setImageType(vk::ImageType::e2D) // TODO: Am I really need this or hardcode it is ok
                .setSharingMode(vk::SharingMode::eExclusive);

            vk::ImageUsageFlags imageUsage = ToVulkanImageUsage(description.initialUsage);
            
            if (true) // TODO
                imageUsage |= vk::ImageUsageFlagBits::eColorAttachment;
                
            if ((imageUsage & vk::ImageUsageFlagBits::eSampled) || (imageUsage & vk::ImageUsageFlagBits::eStorage))
                imageUsage |= (vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst);

            imageCreateInfo.setUsage(imageUsage);

            auto result = vmaCreateImage
            (
                mAllocator,
                reinterpret_cast<const VkImageCreateInfo *>(&imageCreateInfo),
                &allocationCreateInfo, &image, &allocation,
                nullptr
            );

            LOG_TRACE("Image allocate result {}", vk::to_string(vk::Result(result)));
       
            return { image, allocation };
        }

        void FreeImage(Handle image, Allocation allocation) override
        {
            vmaDestroyImage(mAllocator, static_cast<VkImage>(image), static_cast<VmaAllocation>(allocation));
        }

        AllocatedBuffer AllocateBuffer(const BufferDescription& description, MemoryUsage memoryUsage) override
        {
            VmaAllocation allocation;
            VkBuffer buffer;

            VmaAllocationCreateInfo allocationCreateInfo{};
            allocationCreateInfo.usage = static_cast<VmaMemoryUsage>(memoryUsage);

            vk::BufferCreateInfo bufferCreateInfo;
            bufferCreateInfo
                .setSize(description.size)
                .setUsage(ToVulkanBufferUsage(description.bufferUsage))
                .setSharingMode(vk::SharingMode::eExclusive);

            vmaCreateBuffer
            (
                mAllocator,
                reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo),
                &allocationCreateInfo, reinterpret_cast<VkBuffer*>(&buffer), &allocation,
                nullptr
            );

            return { buffer, allocation };
        }

        void FreeBuffer(Handle buffer, Allocation allocation) override
        {
            vmaDestroyBuffer(mAllocator, static_cast<VkBuffer>(buffer), static_cast<VmaAllocation>(allocation));
        }
    };

    Scope<DeviceAllocator> DeviceAllocator::Create(const DeviceAllocatorDescription& description)
    {
        return CreateScope<VulkanAllocator>(description); 
    }
} // namespace Fluent
