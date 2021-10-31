#pragma once

#include "Core/Base.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Image.hpp"
#include "Renderer/Buffer.hpp"

namespace Fluent
{
    using Allocation = Handle;

    struct DeviceAllocatorDescription
    {
        Handle instance;
        Handle physicalDevice;
        Handle device;
    };

    struct AllocatedImage
    {
        Handle image;
        Allocation allocation;
    };

    struct AllocatedBuffer
    {
        Handle buffer;
        Allocation allocation;
    };

    class DeviceAllocator
    {
    protected:
        DeviceAllocator() = default;
    public:
        virtual ~DeviceAllocator() = default;

        virtual AllocatedImage AllocateImage(const ImageDescription& info, MemoryUsage memoryUsage) = 0;
        virtual void FreeImage(Handle image, Allocation allocation) = 0;

        virtual AllocatedBuffer AllocateBuffer(const BufferDescription& description, MemoryUsage memoryUsage) = 0;
        virtual void FreeBuffer(Handle buffer, Allocation allocation) = 0;

        virtual void MapMemory(Allocation allocation, void** data) const = 0;
        virtual void UnmapMemory(Allocation allocation) const = 0;
        virtual void FlushMemory(Allocation allocation, uint32_t size, uint32_t offset) const = 0;

        static Scope<DeviceAllocator> Create(const DeviceAllocatorDescription& description);
    };
} // namespace Fluent
