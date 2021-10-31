#include "Renderer/GraphicContext.hpp"
#include "Renderer/DeviceAllocator.hpp"
#include "Renderer/Buffer.hpp"

namespace Fluent
{
    class VulkanBuffer : public Buffer
    {
    private:
        Allocation mAllocation;
        vk::Buffer mHandle;
        void* mMappedMemory = nullptr;
    public:
        VulkanBuffer(const BufferDescription& description)
            : mAllocation(nullptr)
            , mHandle(nullptr)
            , mMappedMemory(nullptr)
        {
            auto [buffer, allocation] = GetGraphicContext().GetDeviceAllocator()
                .AllocateBuffer(description, description.memoryUsage);
            
            mHandle = (VkBuffer)buffer;
            mAllocation = allocation;
        }

        ~VulkanBuffer() override
        {
            if (mAllocation)
            {
                GetGraphicContext().GetDeviceAllocator().FreeBuffer(mHandle, mAllocation);
            }
        }

        void* MapMemory() override
        {
            if (IsMemoryMapped()) return mMappedMemory;
            GetGraphicContext().GetDeviceAllocator().MapMemory(mAllocation, &mMappedMemory);
            return mMappedMemory;
        }

        void UnmapMemory() override
        {
            if (!IsMemoryMapped()) return;
            GetGraphicContext().GetDeviceAllocator().UnmapMemory(mAllocation);
            mMappedMemory = nullptr;
        }

        void FlushMemory(uint32_t size, uint32_t offset) override
        {
            GetGraphicContext().GetDeviceAllocator().FlushMemory(mAllocation, size, offset);
        }

        bool IsMemoryMapped() const override
        {
            return mMappedMemory;
        }

        void WriteData(const void* data, uint32_t size, uint32_t offset) override
        {
            if (!IsMemoryMapped())
            {
                MapMemory();
                std::memcpy((void*)((uint8_t*)mMappedMemory + offset), data, size);
                UnmapMemory();
            }
            else
            {
                std::memcpy((void*)((uint8_t*)mMappedMemory + offset), data, size);
            }
        }

        Handle GetNativeHandle() const { return mHandle; }
    };

    /// Interface

    Ref<Buffer> Buffer::Create(const BufferDescription& description)
    {
        return CreateRef<VulkanBuffer>(description);
    }
} // namespace Fluent
