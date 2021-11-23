#pragma once

#include "Core/Base.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/RenderPass.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/DescriptorSetLayout.hpp"

namespace Fluent
{
    struct RasterizerStateDescription
    {
        CullMode cullMode;
        FrontFace frontFace;
    };

    struct DepthStateDescription
    {
        bool depthTest;
        bool depthWrite;
        CompareOp compareOp;
    };

    struct PipelineDescription
    {
        PipelineType                            type;
        Ref<DescriptorSetLayout>                descriptorSetLayout;
        std::vector<VertexBindingDescription>   bindingDescriptions;
        std::vector<VertexAttributeDescription> attributeDescriptions;
        RasterizerStateDescription              rasterizerDescription;
        DepthStateDescription                   depthStateDescription;
        Ref<RenderPass>                         renderPass;
    };

    class Pipeline
    {
    protected:
        Pipeline() = default;
    public:
        virtual ~Pipeline() = default;

        virtual PipelineType GetType() const = 0;

        virtual Handle GetPipelineLayout() const = 0;
        virtual Handle GetNativeHandle() const = 0;

        static Ref<Pipeline> Create(const PipelineDescription& description);
    };
} // namespace Fluent
