#pragma once

#include <string>
#include "Core/Base.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/ShaderReflection.hpp"

namespace Fluent
{
    struct ShaderDescription
    {
        std::vector<uint32_t>   byteCode;
        ShaderStage             stage;
        std::string             filename;
        std::vector<ShaderType> inputAttributes;
        std::vector<Uniform>    uniforms;
    };

    class Shader
    {
    protected:
        Shader() = default;
    public:
        virtual ~Shader() = default;

        virtual ShaderStage GetStage() const = 0;
        virtual const std::vector<ShaderUniforms>& GetUniforms() const = 0;
        virtual Handle GetNativeHandle() const = 0;
        
        static Ref<Shader> Create(const ShaderDescription& description);
    };
} // namespace Fluent
