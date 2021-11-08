#include <vulkan/vulkan.hpp>
#include "Renderer/GraphicContext.hpp"
#include "Renderer/Pipeline.hpp"

namespace Fluent
{
    class VulkanPipeline : public Pipeline
    {
    private:
        PipelineType mType;
        vk::Pipeline mHandle;
        vk::PipelineLayout mPipelineLayout;
        const uint32_t mMaxPushConstantRange = 128;

        void InitGraphicsPipeline(const PipelineDescription& description)
        {
            std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos;
            for (auto& shader : description.descriptorSetLayout->GetShaders())
            {
                vk::PipelineShaderStageCreateInfo shaderStageCreateInfo;
                shaderStageCreateInfo
                    .setModule((VkShaderModule)shader->GetNativeHandle())
                    .setPName("main")
                    .setStage(ToVulkanShaderStage(shader->GetStage()));
                shaderStageCreateInfos.push_back(shaderStageCreateInfo);
            }

            std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
            bindingDescriptions.reserve(description.bindingDescriptions.size());
            for (auto& bindingDescription : description.bindingDescriptions)
            {
                bindingDescriptions.emplace_back
                (
                vk::VertexInputBindingDescription
                { 
                    bindingDescription.binding,
                    bindingDescription.stride,
                    ToVulkanVertexInputRate(bindingDescription.inputRate)
                });
            }

            std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
            attributeDescriptions.reserve(description.attributeDescriptions.size());
            for (auto& attributeDescription : description.attributeDescriptions)
            {
                attributeDescriptions.emplace_back
                (
                vk::VertexInputAttributeDescription
                { 
                    attributeDescription.location,
                    attributeDescription.binding,
                    ToVulkanFormat(attributeDescription.format),
                    attributeDescription.offset
                });
            }

            vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
            vertexInputStateCreateInfo
                .setVertexBindingDescriptions(bindingDescriptions)
                .setVertexAttributeDescriptions(attributeDescriptions);
                
            vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
            inputAssemblyStateCreateInfo
                .setTopology(vk::PrimitiveTopology::eTriangleList)
                .setPrimitiveRestartEnable(false);

            // Dynamic states
            vk::Viewport viewport;
            vk::Rect2D scissor;

            vk::PipelineViewportStateCreateInfo viewportStateCreateInfo;
            viewportStateCreateInfo
                .setViewports(viewport)
                .setScissors(scissor);

            vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
            rasterizationStateCreateInfo
                .setPolygonMode(vk::PolygonMode::eFill)
                .setCullMode(ToVulkanCullMode(description.rasterizerDescription.cullMode))
                .setFrontFace(ToVulkanFrontFace(description.rasterizerDescription.frontFace))
                .setLineWidth(1.0f);

            vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
            multisampleStateCreateInfo
                .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                .setMinSampleShading(1.0f);

            vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;
            colorBlendAttachmentState
                .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                .setDstColorBlendFactor(vk::BlendFactor::eZero)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                .setColorWriteMask
                (
                    vk::ColorComponentFlagBits::eR |
                    vk::ColorComponentFlagBits::eG |
                    vk::ColorComponentFlagBits::eB |
                    vk::ColorComponentFlagBits::eA
                );

            vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo;
            colorBlendStateCreateInfo
                .setLogicOpEnable(false)
                .setLogicOp(vk::LogicOp::eCopy)
                .setAttachments(colorBlendAttachmentState)
                .setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

            std::array dynamicStates { vk::DynamicState::eScissor, vk::DynamicState::eViewport };
            vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
            dynamicStateCreateInfo
                .setDynamicStates(dynamicStates);

            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();

            vk::DescriptorSetLayout descriptorSetLayout = (VkDescriptorSetLayout)description.descriptorSetLayout->GetNativeHandle();
            
            vk::PushConstantRange pushConstantRange;
            pushConstantRange
                .setSize(mMaxPushConstantRange)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eFragment);

            vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
            pipelineLayoutCreateInfo.setSetLayouts(descriptorSetLayout);
            pipelineLayoutCreateInfo.setPushConstantRanges(pushConstantRange);

            mPipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

            vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
            pipelineCreateInfo
                .setStages(shaderStageCreateInfos)
                .setPVertexInputState(&vertexInputStateCreateInfo)
                .setPInputAssemblyState(&inputAssemblyStateCreateInfo)
                .setPViewportState(&viewportStateCreateInfo)
                .setPMultisampleState(&multisampleStateCreateInfo)
                .setPRasterizationState(&rasterizationStateCreateInfo)
                .setPColorBlendState(&colorBlendStateCreateInfo)
                .setPDynamicState(&dynamicStateCreateInfo)
                .setLayout(mPipelineLayout)
                .setRenderPass((VkRenderPass)description.renderPass->GetNativeHandle());
            
            mHandle = device.createGraphicsPipeline({}, pipelineCreateInfo).value;
        }

        void InitComputePipeline(const PipelineDescription& description)
        {
            // TODO: check that only one exist
            auto& shader = description.descriptorSetLayout->GetShaders()[0];
            vk::PipelineShaderStageCreateInfo shaderStageCreateInfo;
            shaderStageCreateInfo
                .setModule((VkShaderModule)shader->GetNativeHandle())
                .setPName("main")
                .setStage(ToVulkanShaderStage(shader->GetStage()));

            std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
            bindingDescriptions.reserve(description.bindingDescriptions.size());
            for (auto& bindingDescription : description.bindingDescriptions)
            {
                bindingDescriptions.emplace_back
                (
                vk::VertexInputBindingDescription
                { 
                    bindingDescription.binding,
                    bindingDescription.stride,
                    ToVulkanVertexInputRate(bindingDescription.inputRate)
                });
            }

            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();

            vk::DescriptorSetLayout descriptorSetLayout = (VkDescriptorSetLayout)description.descriptorSetLayout->GetNativeHandle();
            
            vk::PushConstantRange pushConstantRange;
            pushConstantRange
                .setSize(mMaxPushConstantRange)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eFragment);

            vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
            pipelineLayoutCreateInfo.setSetLayouts(descriptorSetLayout);
            pipelineLayoutCreateInfo.setPushConstantRanges(pushConstantRange);

            mPipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

            vk::ComputePipelineCreateInfo computePipelineCreateInfo;
            computePipelineCreateInfo
                .setStage(shaderStageCreateInfo)
                .setLayout(mPipelineLayout);
            
            mHandle = device.createComputePipeline({}, computePipelineCreateInfo).value;
        }
    public:
        VulkanPipeline(const PipelineDescription& description)
            : mType(description.type)
        {
            switch (mType)
            {
                case PipelineType::eGraphics:
                    InitGraphicsPipeline(description);
                    break;
                case PipelineType::eCompute:
                    InitComputePipeline(description);
                    break;
                default:
                    LOG_WARN("Unknown pipeline type {}", uint32_t(mType));
                    break;
            }
        }

        ~VulkanPipeline() override
        {
            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            device.destroyPipelineLayout(mPipelineLayout);
            device.destroyPipeline(mHandle);
        }

        PipelineType GetType() const override { return mType; }
        Handle GetNativeHandle() const override { return mHandle; }
        Handle GetPipelineLayout() const override { return mPipelineLayout; }
    };

    Ref<Pipeline> Pipeline::Create(const PipelineDescription& description)
    {
        return CreateRef<VulkanPipeline>(description);
    }
} // namespace Fluent
