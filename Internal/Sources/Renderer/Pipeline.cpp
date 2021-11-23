#include "Renderer/GraphicContext.hpp"
#include "Renderer/Pipeline.hpp"

namespace Fluent
{
    class VulkanPipeline : public Pipeline
    {
    private:
        PipelineType mType;
        VkPipeline mHandle = VK_NULL_HANDLE;
        VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
        const uint32_t mMaxPushConstantRange = 128;

        void InitGraphicsPipeline(const PipelineDescription& description)
        {
            std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
            for (auto& shader : description.descriptorSetLayout->GetShaders())
            {
                VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
                shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStageCreateInfo.module = (VkShaderModule)shader->GetNativeHandle();
                shaderStageCreateInfo.pName = "main";
                shaderStageCreateInfo.stage = ToVulkanShaderStage(shader->GetStage());
                shaderStageCreateInfos.push_back(shaderStageCreateInfo);
            }

            std::vector<VkVertexInputBindingDescription> bindingDescriptions;
            bindingDescriptions.reserve(description.bindingDescriptions.size());
            for (auto& bindingDescription : description.bindingDescriptions)
            {
                bindingDescriptions.emplace_back
                (
                    VkVertexInputBindingDescription
                    {
                        bindingDescription.binding,
                        bindingDescription.stride,
                        ToVulkanVertexInputRate(bindingDescription.inputRate)
                    }
                );
            }

            std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
            attributeDescriptions.reserve(description.attributeDescriptions.size());
            for (auto& attributeDescription : description.attributeDescriptions)
            {
                attributeDescriptions.emplace_back
                (
                VkVertexInputAttributeDescription
                { 
                    attributeDescription.location,
                    attributeDescription.binding,
                    ToVulkanFormat(attributeDescription.format),
                    attributeDescription.offset
                });
            }

            VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
            vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
            vertexInputStateCreateInfo.pVertexBindingDescriptions = bindingDescriptions.data();
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
            vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

            VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
            inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssemblyStateCreateInfo.primitiveRestartEnable = false;

            // Dynamic states
            VkViewport viewport{};
            VkRect2D scissor{};

            VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
            viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportStateCreateInfo.viewportCount = 1;
            viewportStateCreateInfo.pViewports = &viewport;
            viewportStateCreateInfo.scissorCount = 1;
            viewportStateCreateInfo.pScissors = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
            rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizationStateCreateInfo.cullMode = ToVulkanCullMode(description.rasterizerDescription.cullMode);
            rasterizationStateCreateInfo.frontFace = ToVulkanFrontFace(description.rasterizerDescription.frontFace);
            rasterizationStateCreateInfo.lineWidth = 1.0f;

            VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
            multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisampleStateCreateInfo.minSampleShading = 1.0f;

            VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
            colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                                                     | VK_COLOR_COMPONENT_G_BIT
                                                     | VK_COLOR_COMPONENT_B_BIT
                                                     | VK_COLOR_COMPONENT_A_BIT;

            VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
            colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendStateCreateInfo.logicOpEnable = false;
            colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
            colorBlendStateCreateInfo.attachmentCount = 1;
            colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;

            VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
            depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilStateCreateInfo.depthTestEnable = description.depthStateDescription.depthTest ? VK_TRUE : VK_FALSE;
            depthStencilStateCreateInfo.depthWriteEnable = description.depthStateDescription.depthWrite ? VK_TRUE : VK_FALSE;
            depthStencilStateCreateInfo.depthCompareOp = description.depthStateDescription.depthTest ?
                                                         ToVulkanCompareOp(description.depthStateDescription.compareOp)
                                                         : VK_COMPARE_OP_ALWAYS;
            depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
            depthStencilStateCreateInfo.minDepthBounds = 0.0f; // Optional
            depthStencilStateCreateInfo.maxDepthBounds = 1.0f; // Optional
            depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

            const uint32_t dynamicStatesCount = 2;
            VkDynamicState dynamicStates[dynamicStatesCount] =
                { VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT };

            VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
            dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateCreateInfo.dynamicStateCount = dynamicStatesCount;
            dynamicStateCreateInfo.pDynamicStates = dynamicStates;

            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();

            VkDescriptorSetLayout descriptorSetLayout = (VkDescriptorSetLayout)description.descriptorSetLayout->GetNativeHandle();
            
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.size = mMaxPushConstantRange;
            pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
                                         | VK_SHADER_STAGE_COMPUTE_BIT
                                         | VK_SHADER_STAGE_FRAGMENT_BIT;

            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = 1;
            pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
            pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
            pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

            VK_ASSERT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout));

            VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
            pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineCreateInfo.stageCount = shaderStageCreateInfos.size();
            pipelineCreateInfo.pStages = shaderStageCreateInfos.data();
            pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
            pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
            pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
            pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
            pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
            pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
            pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
            pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
            pipelineCreateInfo.layout = mPipelineLayout;
            pipelineCreateInfo.renderPass = (VkRenderPass)description.renderPass->GetNativeHandle();

            VK_ASSERT(vkCreateGraphicsPipelines(device, {}, 1, &pipelineCreateInfo, nullptr, &mHandle));
        }

        void InitComputePipeline(const PipelineDescription& description)
        {
            // TODO: check that only one exist
            auto& shader = description.descriptorSetLayout->GetShaders()[0];
            VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
            shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCreateInfo.module = (VkShaderModule)shader->GetNativeHandle();
            shaderStageCreateInfo.pName = "main";
            shaderStageCreateInfo.stage = ToVulkanShaderStage(shader->GetStage());

            std::vector<VkVertexInputBindingDescription> bindingDescriptions;
            bindingDescriptions.reserve(description.bindingDescriptions.size());
            for (auto& bindingDescription : description.bindingDescriptions)
            {
                bindingDescriptions.emplace_back
                (
                VkVertexInputBindingDescription
                { 
                    bindingDescription.binding,
                    bindingDescription.stride,
                    ToVulkanVertexInputRate(bindingDescription.inputRate)
                });
            }

            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();

            VkDescriptorSetLayout descriptorSetLayout = (VkDescriptorSetLayout)description.descriptorSetLayout->GetNativeHandle();
            
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.size = mMaxPushConstantRange;
            pushConstantRange.stageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = 1;
            pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
            pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
            pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

            VK_ASSERT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout));

            VkComputePipelineCreateInfo computePipelineCreateInfo{};
            computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            computePipelineCreateInfo.stage = shaderStageCreateInfo;
            computePipelineCreateInfo.layout = mPipelineLayout;

            VK_ASSERT
            (vkCreateComputePipelines
                (device, {}, 1,
                 &computePipelineCreateInfo,
                 nullptr, &mHandle)
             );
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
            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            vkDestroyPipelineLayout(device, mPipelineLayout, nullptr);
            vkDestroyPipeline(device, mHandle, nullptr);
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
