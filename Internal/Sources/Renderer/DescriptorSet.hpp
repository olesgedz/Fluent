#pragma once

#include "Core/Base.hpp"
#include "Renderer/DescriptorSetLayout.hpp"

namespace Fluent
{
    struct DescriptorSetDescription
    {
        Ref<DescriptorSetLayout> descriptorSetLayout;
    };

    struct BufferUpdateDesc
    {
        Ref<Buffer> buffer;
        uint32_t offset = 0;
        uint32_t range = 0;
    };

    struct DescriptorUpdateDesc
    {
        DescriptorType descriptorType;
        uint32_t binding;
        BufferUpdateDesc bufferUpdate;
    };

    class DescriptorSet
    {
    protected:
        DescriptorSet() = default;
    public:
        virtual ~DescriptorSet() = default;

        virtual void UpdateDescriptorSet(const DescriptorUpdateDesc& bufferUpdateDesc) = 0;

        virtual Handle GetNativeHandle() const = 0;
         
        static Ref<DescriptorSet> Create(const DescriptorSetDescription& description);
    };
} // namespace Fluent
