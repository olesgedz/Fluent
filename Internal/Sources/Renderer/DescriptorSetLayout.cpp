#include "Renderer/GraphicContext.hpp"
#include "Renderer/DescriptorSetLayout.hpp"

namespace Fluent
{
    class VulkanDescriptorSetLayout : public DescriptorSetLayout
    {
    private:
        VkDescriptorSetLayout mHandle = VK_NULL_HANDLE;
        std::vector<Ref<Shader>> mShaders;
    public:
        VulkanDescriptorSetLayout(const DescriptorSetLayoutDescription& description)
            : mShaders(description.shaders)
        {
            std::vector<VkDescriptorSetLayoutBinding> bindings;

            size_t totalUniformCount = 0;
            for (const auto& shader : description.shaders)
            {
                for (const auto& uniformsPerShader : shader->GetUniforms())
                    totalUniformCount += uniformsPerShader.uniforms.size();
            }

            bindings.reserve(totalUniformCount);
            for (const auto& shader : description.shaders)
            {
                for (const auto& uniformsPerShader : shader->GetUniforms())
                {
                    for (const auto& uniform : uniformsPerShader.uniforms)
                    {
                        auto layoutIt = std::find_if(bindings.begin(), bindings.end(),
                            [&uniform](const auto& layout) { return layout.binding == uniform.binding; });

                        if (layoutIt != bindings.end())
                        {
                            layoutIt->stageFlags |= ToVulkanShaderStage(uniformsPerShader.stage);
                            continue; // do not add new binding
                        }

                        bindings.push_back(VkDescriptorSetLayoutBinding
                        {
                            uniform.binding,
                            ToVulkanDescriptorType(uniform.descriptorType),
                            uniform.descriptorCount,
                            ToVulkanShaderStage(shader->GetStage())
                        });
                    }
                }
            }

            VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
            layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutCreateInfo.pBindings = bindings.data();

            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            VK_ASSERT(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &mHandle));
        }

        ~VulkanDescriptorSetLayout() override
        {
            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            vkDestroyDescriptorSetLayout(device, mHandle, nullptr);
        }

        const std::vector<Ref<Shader>>& GetShaders() const override { return mShaders; }
        Handle GetNativeHandle() const override { return mHandle; }
    };

    Ref<DescriptorSetLayout> DescriptorSetLayout::Create(const DescriptorSetLayoutDescription& description)
    {
        return CreateRef<VulkanDescriptorSetLayout>(description);
    }
} // namespace Fluent
