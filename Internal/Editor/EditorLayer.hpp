#pragma once

#include "Core/Base.hpp"
#include "Core/Layer.hpp"

namespace Fluent
{
    class RenderPass;
    class UIContext;
    class EditorLayer : public Layer
    {
    private:
        Ref<RenderPass>     mRenderPass;
        Scope<UIContext>    mUIContext;
    public:
        EditorLayer() : Layer("Fluent editor") {}
        ~EditorLayer() override = default;

        void OnAttach() override;
        void OnLoad() override;
        void OnUnload() override;
        void OnDetach() override;
        void OnUpdate(float deltaTime) override;
    };
}