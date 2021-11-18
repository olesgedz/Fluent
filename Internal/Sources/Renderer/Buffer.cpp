#include "Renderer/GraphicContext.hpp"
#include "Renderer/DeviceAllocator.hpp"
#include "Renderer/Buffer.hpp"

namespace Fluent
{
    class VulkanBuffer : public Buffer
    {
    private:
        Allocation mAllocation;
        VkBuffer mHandle;
        uint32_t mSize;
        void* mMappedMemory = nullptr;

        void InitBuffer(const BufferDescription& description)
        {
            auto& context = GetGraphicContext();
            // Staging
            if (description.data && description.memoryUsage == MemoryUsage::eGpu)
            {
                auto stage = context.GetStagingBuffer()->Submit(description.data, description.size);
                auto [buffer, allocation] = context.GetDeviceAllocator().AllocateBuffer(description, description.memoryUsage);

                mHandle = (VkBuffer)buffer;
                mAllocation = allocation;

                auto& cmd = context.GetCurrentCommandBuffer();
                cmd->Begin();
                cmd->CopyBuffer(context.GetStagingBuffer()->GetBuffer(), stage.offset, *this, 0, description.size);
                cmd->End();
                context.ImmediateSubmit(cmd);
            }
            else
            {
                auto [buffer, allocation] = context.GetDeviceAllocator().AllocateBuffer(description, description.memoryUsage);
                mHandle = (VkBuffer)buffer;
                mAllocation = allocation;
            }
        }
    public:
        VulkanBuffer(const BufferDescription& description)
            : mAllocation(nullptr)
            , mHandle(nullptr)
            , mMappedMemory(nullptr)
            , mSize(description.size)
        {
            InitBuffer(description);
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

        uint32_t GetSize() const override { return mSize; }
        Handle GetNativeHandle() const override { return mHandle; }
    };

    /// Interface

    Ref<Buffer> Buffer::Create(const BufferDescription& description)
    {
        return CreateRef<VulkanBuffer>(description);
    }
} // namespace Fluent
