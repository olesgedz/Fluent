#pragma once

#include "Core/Base.hpp"

namespace Fluent
{
    struct UIContextDescription
    {
        Ref<RenderPass> renderPass;
    };

    class UIContext
    {
    protected:
        UIContext() = default;
    public:
        virtual ~UIContext() = default;

        virtual void BeginFrame() const = 0;
        virtual void EndFrame() const = 0;
        
        static Scope<UIContext> Create(const UIContextDescription& description);
    };
} // namespace Fluent
