#pragma once

#include <cstdint>
#include "Renderer/Renderer.hpp"

namespace Fluent
{
    struct ShaderDescription;
    
    struct ShaderType
    {
        Format format;
        uint32_t componentCount;
        uint32_t byteSize;
    };

    struct Uniform
    {
        DescriptorType descriptorType;
        uint32_t       binding;
        uint32_t       descriptorCount = 1;
    };

    struct ShaderUniforms
    {
        ShaderStage stage;
        std::vector<Uniform> uniforms;
    };

    void Reflect(ShaderDescription& description);
} // namespace Fluent
