#include "Renderer/GraphicContext.hpp"
#include "Renderer/DescriptorSetLayout.hpp"

namespace Fluent
{
    class VulkanDescriptorSetLayout : public DescriptorSetLayout
    {
    private:
        vk::DescriptorSetLayout mHandle;
        std::vector<Ref<Shader>> mShaders;
    public:
        VulkanDescriptorSetLayout(const DescriptorSetLayoutDescription& description)
            : mShaders(description.shaders)
        {
            std::vector<vk::DescriptorSetLayoutBinding> bindings;
            std::vector<vk::DescriptorBindingFlags> bindingFlags;

            size_t totalUniformCount = 0;
            for (const auto& shader : description.shaders)
            {
                for (const auto& uniformsPerShader : shader->GetUniforms())
                    totalUniformCount += uniformsPerShader.uniforms.size();
            }

            bindings.reserve(totalUniformCount);
            bindingFlags.reserve(totalUniformCount);

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

                        bindings.push_back(vk::DescriptorSetLayoutBinding
                        {
                            uniform.binding,
                            ToVulkanDescriptorType(uniform.descriptorType),
                            uniform.descriptorCount,
                            ToVulkanShaderStage(shader->GetStage())
                        });

                        vk::DescriptorBindingFlags descriptorBindingFlags = { };
                        // descriptorBindingFlags |= vk::DescriptorBindingFlagBits::eUpdateAfterBind;
                        if (uniform.descriptorCount > 1)
                            descriptorBindingFlags |= vk::DescriptorBindingFlagBits::ePartiallyBound;
                        bindingFlags.push_back(descriptorBindingFlags);
                    }
                }
            }

            vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo;
            bindingFlagsCreateInfo.setBindingFlags(bindingFlags);

            vk::DescriptorSetLayoutCreateInfo layoutCreateInfo;
            layoutCreateInfo.setFlags(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool);
            layoutCreateInfo.setBindings(bindings);
            layoutCreateInfo.setPNext(&bindingFlagsCreateInfo);

            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            mHandle = device.createDescriptorSetLayout(layoutCreateInfo);
        }

        ~VulkanDescriptorSetLayout() override
        {
            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            device.destroyDescriptorSetLayout(mHandle);
        }

        const std::vector<Ref<Shader>>& GetShaders() const override { return mShaders; }
        Handle GetNativeHandle() const override { return mHandle; }
    };

    Ref<DescriptorSetLayout> DescriptorSetLayout::Create(const DescriptorSetLayoutDescription& description)
    {
        return CreateRef<VulkanDescriptorSetLayout>(description);
    }
} // namespace Fluent
