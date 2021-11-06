#pragma once
#include <cstdint>
#include "Core/Base.hpp"
#include "Renderer/Buffer.hpp"

namespace Fluent
{
    struct StagingBufferDescription
    {
        uint32_t size;
    };

    class StagingBuffer
    {
    protected:
        StagingBuffer() = default;
    public:
        virtual ~StagingBuffer() = default;

        struct StagingAllocation
        {
            uint32_t size;
            uint32_t offset;
        };

		virtual StagingAllocation Submit(const void* data, uint32_t byteSize) = 0;

        virtual void Flush() = 0;
        virtual void Reset() = 0;

        virtual Ref<Buffer> GetBuffer() const = 0;
        virtual uint32_t GetCurrentOffset() const = 0;

        static Ref<StagingBuffer> Create(const StagingBufferDescription& description);
    };
} // namespace Fluent
