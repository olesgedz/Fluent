#include <fstream>
#include <vector>
#include "Core/FileSystem.hpp"
#include "Renderer/GraphicContext.hpp"
#include "Renderer/Shader.hpp"

namespace Fluent
{
    class VulkanShader : public Shader
    {
    protected:
        ShaderStage                 mShaderStage;
        VkShaderModule              mHandle;
        std::vector<ShaderType>     mInputAttributes;
        std::vector<ShaderUniforms> mUniforms;

        static std::vector<uint32_t> ReadSpirvBytecode(const std::string& filepath)
        {
            std::ifstream file(filepath, std::ios_base::binary);
            if (!file.is_open())
                LOG_WARN("Failed to open file {}", filepath);

            std::vector<char> res = { std::istreambuf_iterator(file), std::istreambuf_iterator<char>() };
            return std::vector<uint32_t>(reinterpret_cast<uint32_t*>(res.data()), reinterpret_cast<uint32_t*>(res.data() + res.size()));
        }

        static ShaderDescription LoadShader(const ShaderDescription& description)
        {
            ShaderDescription result = description;
            result.byteCode = ReadSpirvBytecode(FileSystem::GetShadersDirectory() + description.filename + ".spv");
            return Reflect(result);
        }
    public:
        VulkanShader(const ShaderDescription& description)
            : mShaderStage(description.stage)
        {
            auto loadedDescription = LoadShader(description);
            
            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();

            VkShaderModuleCreateInfo shaderCreateInfo{};
            shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shaderCreateInfo.codeSize = loadedDescription.byteCode.size() * sizeof(uint32_t);
            shaderCreateInfo.pCode = loadedDescription.byteCode.data();

            VK_ASSERT(vkCreateShaderModule(device, &shaderCreateInfo, nullptr, &mHandle));

            if (loadedDescription.stage == ShaderStage::eVertex)
                mInputAttributes = loadedDescription.inputAttributes;

            auto& uniforms = mUniforms.emplace_back();
            uniforms.stage = loadedDescription.stage;
            uniforms.uniforms.insert(uniforms.uniforms.end(), loadedDescription.uniforms.begin(), loadedDescription.uniforms.end());
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
