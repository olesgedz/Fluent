#pragma once

#include <span>
#include "Core/Base.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/Image.hpp"
#include "Renderer/Sampler.hpp"
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

    struct ImageUpdateDesc
    {
        Ref<Image>          image;
        Ref<Sampler>        sampler;
        ImageUsage::Bits    usage;
    };

    struct DescriptorSetUpdateDesc
    {
        DescriptorType      descriptorType;
        uint32_t            binding;
        BufferUpdateDesc    bufferUpdate;
        ImageUpdateDesc     imageUpdate;
    };

    class DescriptorSet
    {
    protected:
        DescriptorSet() = default;
    public:
        virtual ~DescriptorSet() = default;

        virtual void UpdateDescriptorSet(const std::span<DescriptorSetUpdateDesc>& updateDesc) = 0;

        virtual Handle GetNativeHandle() const = 0;
         
        static Ref<DescriptorSet> Create(const DescriptorSetDescription& description);
    };
} // namespace Fluent
