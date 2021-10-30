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
    public:
        VulkanBuffer(const BufferDescription& description)
            : mAllocation(nullptr)
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

        Handle GetNativeHandle() const { return mHandle; }
    };

    /// Interface

    Ref<Buffer> Buffer::Create(const BufferDescription& description)
    {
        return CreateRef<VulkanBuffer>(description);
    }
} // namespace Fluent
