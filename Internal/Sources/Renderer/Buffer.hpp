#pragma once

#include "Core/Base.hpp"
#include "Renderer/Renderer.hpp"

namespace Fluent
{
    struct BufferDescription
    {
        MemoryUsage         memoryUsage;
        BufferUsage::Bits   bufferUsage; // TODO: Can be changed to descriptor type
        uint32_t            size;
    };
    
    class Buffer
    {
    protected:
        Buffer() = default;
    public:
        virtual ~Buffer() = default;
        
        virtual Handle GetNativeHandle() const = 0;

        static Ref<Buffer> Create(const BufferDescription& description);
    };
} // namespace Fluent
