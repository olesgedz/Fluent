#pragma once

#include <vector>
#include "Renderer/Image.hpp"
#include "Renderer/RenderPass.hpp"

namespace Fluent
{
    struct FramebufferDescription
    {
        uint32_t width;
        uint32_t height;
        Ref<RenderPass> renderPass;
        std::vector<Ref<Image>> targets;
        Ref<Image> depthStencil;
    };

    class Framebuffer
    {
    protected:
        Framebuffer() = default;
    public:
        virtual ~Framebuffer() = default;

        virtual Handle GetNativeHandle() const = 0;
         
        static Ref<Framebuffer> Create(const FramebufferDescription& description);
    };
} // namespace Fluent
