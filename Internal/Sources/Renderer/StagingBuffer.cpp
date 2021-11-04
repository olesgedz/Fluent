#include "Renderer/StagingBuffer.hpp"

namespace Fluent
{
    class VulkanStagingBuffer : public StagingBuffer
    {
    private:
        Ref<Buffer> mBuffer;
        uint32_t mCurrentOffset;
    public:
        VulkanStagingBuffer(const StagingBufferDescription& description)
            : mBuffer(nullptr)
            , mCurrentOffset(0)
        {
            BufferDescription bufferDesc{};
            bufferDesc.bufferUsage = BufferUsage::eTransferSrc;
            bufferDesc.memoryUsage = MemoryUsage::eCpu;
            bufferDesc.size = description.size;

            mBuffer = Buffer::Create(bufferDesc);
            mBuffer->MapMemory();
        }

        ~VulkanStagingBuffer() override
        {
            mBuffer->UnmapMemory();
            mBuffer = nullptr;
        }

		StagingAllocation Submit(const void* data, uint32_t byteSize) override
        {
            if (mCurrentOffset + byteSize > mBuffer->GetSize())
            {
                LOG_ERROR
                (
                    "Staging buffer free space less than data to write. Write size {} Buffer Size {} Free Space {}", 
                    byteSize, mBuffer->GetSize(), mBuffer->GetSize() - mCurrentOffset
                );
            }

            if (data != nullptr)
            {
                mBuffer->WriteData(data, byteSize, mCurrentOffset);
            }

            mCurrentOffset += byteSize;
            return StagingAllocation{ byteSize, mCurrentOffset - byteSize };
        }

        void Flush() override
        {
            mBuffer->FlushMemory(mCurrentOffset, 0);
        }

        void Reset() override
        {
            mCurrentOffset = 0;
        }

        Ref<Buffer> GetBuffer() const override
        {
            return mBuffer;
        }

        uint32_t GetCurrentOffset() const override
        {
            return mCurrentOffset;
        }
    };

    Ref<StagingBuffer> StagingBuffer::Create(const StagingBufferDescription& description)
    {
        return CreateRef<VulkanStagingBuffer>(description);
    }
} // namespace Fluent
