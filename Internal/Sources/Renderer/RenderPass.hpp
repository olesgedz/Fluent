#pragma once

#include <cstdint>
#include <vector>
#include "Core/Base.hpp"
#include "Renderer/Renderer.hpp"

namespace Fluent
{
    struct ClearValue
    {
        float r, g, b, a;
        float depth;
        uint32_t stencil;
    };

    struct RenderPassDescription
    {
        uint32_t width = 0;
        uint32_t height = 0;
        std::vector<ClearValue> clearValues;
        std::vector<Format> colorFormats;
        std::vector<ImageUsage::Bits> initialUsages;
        std::vector<ImageUsage::Bits> finalUsages;
        std::vector<AttachmentLoadOp> attachmentLoadOps;
        Format depthStencilFormat;
        AttachmentLoadOp depthLoadOp;
        AttachmentLoadOp stencilLoadOp;
		SampleCount sampleCount = SampleCount::e1;
    };

    class RenderPass
    {
    protected:
        RenderPass() = default;
    public:
        virtual ~RenderPass() = default;
        
        virtual void SetRenderArea(uint32_t width, uint32_t height) = 0;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        virtual const std::vector<ClearValue>& GetClearValues() const = 0;

        virtual Handle GetNativeHandle() const = 0;

        static Ref<RenderPass> Create(const RenderPassDescription& description);
    };
} // namespace Fluent
