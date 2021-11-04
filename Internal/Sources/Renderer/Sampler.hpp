#pragma once

#include "Core/Base.hpp"
#include "Renderer/Renderer.hpp"

namespace Fluent
{
    struct SamplerDescription
    {
        Filter magFilter = Filter::eNearest;
        Filter minFilter = Filter::eNearest;
        SamplerMipmapMode  mipmapMode   = SamplerMipmapMode::eNearest;
        SamplerAddressMode addressModeU = SamplerAddressMode::eRepeat;
        SamplerAddressMode addressModeV = SamplerAddressMode::eRepeat;
        SamplerAddressMode addressModeW = SamplerAddressMode::eRepeat;
        float mipLodBias = {};
        bool anisotropyEnable = {};
        float maxAnisotropy = {};
        bool compareEnable = {};
        CompareOp compareOp = CompareOp::eNever;
        float minLod = {};
        float maxLod = {};
    };
    
    class Sampler
    {
    protected:
        Sampler() = default;
    public:
        virtual ~Sampler() = default;

        virtual Handle GetNativeHandle() const = 0;
        
        static Ref<Sampler> Create(const SamplerDescription& description); 
    };
} // namespace Fluent
