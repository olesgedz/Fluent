#pragma once

#include "Core/Base.hpp"
#include "Renderer/Renderer.hpp"

namespace Fluent
{
    struct BufferDescription
    {
        MemoryUsage         memoryUsage;
        BufferUsage::Bits   bufferUsage; // TODO: Can be changed to descriptor type
        void*               data;
        uint32_t            size;
    };
    
    class Buffer
    {
    protected:
        Buffer() = default;
    public:
        virtual ~Buffer() = default;
        
        virtual void WriteData(const void* data, uint32_t size, uint32_t offset) = 0;
        virtual void* MapMemory() = 0;
        virtual void UnmapMemory() = 0;
        virtual void FlushMemory(uint32_t size, uint32_t offset) = 0;

        virtual bool IsMemoryMapped() const = 0;
        
        virtual uint32_t GetSize() const = 0;
        virtual Handle GetNativeHandle() const = 0;

        static Ref<Buffer> Create(const BufferDescription& description);
    };
} // namespace Fluent
