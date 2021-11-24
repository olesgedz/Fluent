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
            std::vector<VkDescriptorBindingFlags> bindingFlags;

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

                        auto& binding = bindings.emplace_back();
                        binding.binding = uniform.binding;
                        binding.descriptorType = ToVulkanDescriptorType(uniform.descriptorType);
                        binding.descriptorCount = uniform.descriptorCount;
                        binding.stageFlags = ToVulkanShaderStage(shader->GetStage());

                        VkDescriptorBindingFlags descriptorBindingFlags = { };
                        if (uniform.descriptorCount > 1)
                            descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                        bindingFlags.push_back(descriptorBindingFlags);
                    }
                }
            }

            VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo{};
            bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
            bindingFlagsCreateInfo.bindingCount = bindingFlags.size();
            bindingFlagsCreateInfo.pBindingFlags = bindingFlags.data();

            VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
            layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutCreateInfo.pBindings = bindings.data();
            layoutCreateInfo.pNext = &bindingFlagsCreateInfo;

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
