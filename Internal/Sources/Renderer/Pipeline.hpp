#pragma once

#include "Core/Base.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/RenderPass.hpp"
#include "Renderer/Shader.hpp"

namespace Fluent
{
    struct RasterizerStateDescription
    {
        CullMode cullMode;
        FrontFace frontFace;
    };

    struct PipelineDescription
    {
        PipelineType                            type;
        std::vector<VertexBindingDescription>   bindingDescriptions;
        std::vector<VertexAttributeDescription> attributeDescriptions;
        RasterizerStateDescription              rasterizerDescription;
        std::vector<Ref<Shader>>                shaders;
        Ref<RenderPass>                         renderPass;
    };

    class Pipeline
    {
    protected:
        Pipeline() = default;
    public:
        virtual ~Pipeline() = default;

        virtual PipelineType GetType() const = 0;

        virtual Handle GetNativeHandle() const = 0;

        static Ref<Pipeline> Create(const PipelineDescription& description);
    };
} // namespace Fluent
