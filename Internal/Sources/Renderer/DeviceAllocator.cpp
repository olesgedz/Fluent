#include "Renderer/Renderer.hpp"
#include <vk_mem_alloc.h>
#include "Renderer/DeviceAllocator.hpp"

namespace Fluent
{
    class VulkanAllocator : public DeviceAllocator
    {
    private:
        VkInstance        mInstance;
        VkPhysicalDevice  mPhysicalDevice;
        VkDevice          mDevice;
        VmaAllocator        mAllocator;
    public:
        VulkanAllocator(const DeviceAllocatorDescription& description)
            : mInstance(static_cast<VkInstance>(description.instance))
            , mPhysicalDevice(static_cast<VkPhysicalDevice>(description.physicalDevice))
            , mDevice(static_cast<VkDevice>(description.device))
            , mAllocator(nullptr)
        {
            VmaAllocatorCreateInfo allocatorCreateInfo{};
            allocatorCreateInfo.vulkanApiVersion    = FLUENT_VK_API_VERSION;
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

            VkImageCreateInfo imageCreateInfo{};
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageCreateInfo.arrayLayers = description.arraySize;
            imageCreateInfo.format = ToVulkanFormat(description.format);
            imageCreateInfo.extent = { description.width, description.height, description.depth };
            imageCreateInfo.mipLevels = description.mipLevels;
            imageCreateInfo.samples = ToVulkanSampleCount(description.sampleCount);
            imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCreateInfo.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
            imageCreateInfo.imageType = VkImageType::VK_IMAGE_TYPE_2D;
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VkImageUsageFlags imageUsage = ToVulkanImageUsage(description.initialUsage);
            
            if (!IsDepthFormat(description.format))
                imageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            else
                imageUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                
            if ((imageUsage & VK_IMAGE_USAGE_SAMPLED_BIT) || (imageUsage & VK_IMAGE_USAGE_STORAGE_BIT))
                imageUsage |= (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

            imageCreateInfo.usage = imageUsage;

            auto result = vmaCreateImage
            (
                mAllocator,
                reinterpret_cast<const VkImageCreateInfo *>(&imageCreateInfo),
                &allocationCreateInfo, &image, &allocation,
                nullptr
            );

            VK_ASSERT(result);
       
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

            VkBufferCreateInfo bufferCreateInfo{};
            bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.pNext = nullptr;
            bufferCreateInfo.flags = 0;
            bufferCreateInfo.size = description.size;
            bufferCreateInfo.usage = (VkBufferUsageFlags)ToVulkanBufferUsage(description.bufferUsage);
            bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            bufferCreateInfo.queueFamilyIndexCount = 0;
            bufferCreateInfo.pQueueFamilyIndices = nullptr;


            // TODO!!! We need it when we want staging but it's awful way to determine it
            // I have done it because we can't now translate more than one BufferUsage 
            // I mean we can't translate (VertexBuffer | TransferDst)
            if (memoryUsage == MemoryUsage::eGpu)
                bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                
            auto result = vmaCreateBuffer
            (
                mAllocator,
                &bufferCreateInfo,
                &allocationCreateInfo, &buffer, &allocation,
                nullptr
            );

            // TODO: ASSERT()
            if (result != VK_SUCCESS)
                LOG_ERROR("Buffer allocation failed with result {}", result);

            return { buffer, allocation };
        }

        void FreeBuffer(Handle buffer, Allocation allocation) override
        {
            vmaDestroyBuffer(mAllocator, static_cast<VkBuffer>(buffer), static_cast<VmaAllocation>(allocation));
        }

        void MapMemory(Allocation allocation, void** data) const override
        {
            vmaMapMemory(mAllocator, (VmaAllocation)allocation, data);
        }

        void UnmapMemory(Allocation allocation) const override
        {
            vmaUnmapMemory(mAllocator, (VmaAllocation)allocation);
        }

        void FlushMemory(Allocation allocation, uint32_t size, uint32_t offset) const override
        {
            vmaFlushAllocation(mAllocator, (VmaAllocation)allocation, offset, size);
        }
    };

    Scope<DeviceAllocator> DeviceAllocator::Create(const DeviceAllocatorDescription& description)
    {
        return CreateScope<VulkanAllocator>(description); 
    }
} // namespace Fluent
