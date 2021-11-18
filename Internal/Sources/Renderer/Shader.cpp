#include <fstream>
#include <vector>
#include <shaderc/shaderc.hpp>
#include "Core/FileSystem.hpp"
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
            default: break;
        }
        return shaderc_shader_kind(-1);
    }

    std::vector<uint32_t> CompileShader(const std::string& filepath, ShaderStage stage)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
            LOG_ERROR("File not found {}", filepath);
        std::string code { std::istreambuf_iterator(file), std::istreambuf_iterator<char>() };
        shaderc::Compiler compiler;
        shaderc::CompilationResult module = compiler.CompileGlslToSpv(code.c_str(), code.size(), ShaderStageToShadercType(stage), "name");

        return { module.cbegin(), module.cend() };
    }

    class VulkanShader : public Shader
    {
    protected:
        ShaderStage                 mShaderStage;
        VkShaderModule              mHandle;
        std::vector<ShaderType>     mInputAttributes;
        std::vector<ShaderUniforms> mUniforms;
    public:
        VulkanShader(const ShaderDescription& description)
            : mShaderStage(description.stage)
        {
            ShaderDescription desc = description;
            if (desc.byteCode.empty())
            {
                desc.byteCode = CompileShader(FileSystem::GetShadersDirectory() + desc.filename, desc.stage);
            }
            
            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();

            VkShaderModuleCreateInfo shaderCreateInfo{};
            shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shaderCreateInfo.codeSize = desc.byteCode.size() * sizeof(uint32_t);
            shaderCreateInfo.pCode = desc.byteCode.data();

            VK_ASSERT(vkCreateShaderModule(device, &shaderCreateInfo, nullptr, &mHandle));
            Reflect(desc);

            if (desc.stage == ShaderStage::eVertex)
                mInputAttributes = desc.inputAttributes;

            auto& uniforms = mUniforms.emplace_back();
            uniforms.stage = desc.stage;
            uniforms.uniforms.insert(uniforms.uniforms.end(), desc.uniforms.begin(), desc.uniforms.end());
        } 

        ~VulkanShader() override
        {
            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            vkDestroyShaderModule(device, mHandle, nullptr);
        }

        const std::vector<ShaderUniforms>& GetUniforms() const override { return mUniforms; }
        ShaderStage GetStage() const override { return mShaderStage; }
        Handle GetNativeHandle() const override { return mHandle; }
    };

    /// Interface

    Ref<Shader> Shader::Create(const ShaderDescription& description)
    {
        return CreateRef<VulkanShader>(description);
    }
} // namespace Fluent
