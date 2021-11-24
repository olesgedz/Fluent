#pragma once

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
        DescriptorType                  descriptorType;
        uint32_t                        binding;
        std::vector<BufferUpdateDesc>   bufferUpdates;
        std::vector<ImageUpdateDesc>    imageUpdates;
    };

    class DescriptorSet
    {
    protected:
        DescriptorSet() = default;
    public:
        virtual ~DescriptorSet() = default;

        // TODO: rewrite
        virtual void UpdateDescriptorSet(const std::vector<DescriptorSetUpdateDesc>& updateDesc) = 0;

        virtual Handle GetNativeHandle() const = 0;
         
        static Ref<DescriptorSet> Create(const DescriptorSetDescription& description);
    };
} // namespace Fluent
