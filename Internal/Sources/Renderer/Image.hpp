#pragma once

#include <string>
#include "Core/Base.hpp"
#include "Renderer/Renderer.hpp"

namespace Fluent
{
    struct ImageDescription
    {
        Handle              handle = nullptr;
        uint32_t            arraySize = 0;
        uint32_t            depth = 0;
        Format              format = Format::eUndefined;
        uint32_t            width = 0;
        uint32_t            height = 0;
        uint32_t            mipLevels = 0;
        SampleCount         sampleCount = SampleCount::e1;
        ImageUsage::Bits    initialUsage = ImageUsage::eUndefined;
        std::string         filename;
    };

    class Image
    {
    protected:
        Image() = default;
    public:
        virtual ~Image() = default;

        virtual Format GetFormat() const = 0;
        virtual Handle GetNativeHandle() const = 0;
        virtual Handle GetImageView() const = 0;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        
        static Ref<Image> Create(const ImageDescription& description);
    };
} // namespace Fluent
