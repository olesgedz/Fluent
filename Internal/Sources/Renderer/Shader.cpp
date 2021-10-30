#include <fstream>
#include <vector>
#include <shaderc/shaderc.hpp>
#include "Renderer/GraphicContext.hpp"
#include "Renderer/Shader.hpp"

namespace Fluent
{
    shaderc_shader_kind ShaderStageToShadercType(ShaderStage stage)
    {
        switch (stage)
        {
            case ShaderStage::eVertex: return shaderc_shader_kind::shaderc_vertex_shader;
            case ShaderStage::eTessellationControl: return shaderc_shader_kind::shaderc_tess_control_shader;
            case ShaderStage::eTessellationEvaluation: return shaderc_shader_kind::shaderc_tess_evaluation_shader;
            case ShaderStage::eGeometry: return shaderc_shader_kind::shaderc_geometry_shader;
            case ShaderStage::eFragment: return shaderc_shader_kind::shaderc_fragment_shader;
            case ShaderStage::eCompute: return shaderc_shader_kind::shaderc_compute_shader;
        }
        return shaderc_shader_kind(-1);
    }

    std::vector<uint32_t> CompileShader(const std::string& filename, ShaderStage stage)
    {
        std::ifstream file(filename);
        if (!file.is_open())
            LOG_ERROR("File not found {}", filename);
        std::string code { std::istreambuf_iterator(file), std::istreambuf_iterator<char>() };
        shaderc::Compiler compiler;
        shaderc::CompilationResult module = compiler.CompileGlslToSpv(code.c_str(), code.size(), ShaderStageToShadercType(stage), "name");

        return { module.cbegin(), module.cend() };
    }

    class VulkanShader : public Shader
    {
    protected:
        ShaderStage                 mShaderStage;
        vk::ShaderModule            mHandle;
        std::vector<ShaderType>     mInputAttributes;
        std::vector<ShaderUniforms> mUniforms;
    public:
        VulkanShader(const ShaderDescription& description)
            : mShaderStage(description.stage)
        {
            ShaderDescription desc = description;
            if (desc.byteCode.empty())
            {
                desc.byteCode = CompileShader(desc.filename, desc.stage);
            }
            
            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();

            vk::ShaderModuleCreateInfo shaderCreateInfo;
            shaderCreateInfo
                    .setCodeSize(desc.byteCode.size() * sizeof(uint32_t))
                    .setPCode(desc.byteCode.data());

            mHandle = device.createShaderModule(shaderCreateInfo);

            Reflect(desc);

            if (desc.stage == ShaderStage::eVertex)
                mInputAttributes = desc.inputAttributes;

            auto& uniforms = mUniforms.emplace_back();
            uniforms.stage = desc.stage;
            uniforms.uniforms.insert(uniforms.uniforms.end(), desc.uniforms.begin(), desc.uniforms.end());
        } 

        ~VulkanShader() override
        {
            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            device.destroyShaderModule(mHandle);
        }

        ShaderStage GetStage() const override { return mShaderStage; }
        Handle GetNativeHandle() const override { return mHandle; }
    };

    /// Interface

    Ref<Shader> Shader::Create(const ShaderDescription& description)
    {
        return CreateRef<VulkanShader>(description);
    }
} // namespace Fluent
