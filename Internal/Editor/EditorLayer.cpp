#include "Fluent/Fluent.hpp"
#include "EditorLayer.hpp"

namespace Fluent
{
    void EditorLayer::OnAttach()
    {
        mRenderPass = GetGraphicContext().GetDefaultRenderPass();

        UIContextDescription uiDesc{};
        uiDesc.viewports = true;
        uiDesc.docking = true;
        uiDesc.renderPass = mRenderPass;
        mUIContext = UIContext::Create(uiDesc);
    }

    void EditorLayer::OnLoad()
    {
        auto& window = Application::Get().GetWindow();

        mRenderPass->SetRenderArea(window->GetWidth(), window->GetHeight());
    }

    void EditorLayer::OnUnload()
    {
    }

    void EditorLayer::OnDetach()
    {
        mUIContext = nullptr;
    }

    void EditorLayer::OnUpdate(float deltaTime)
    {
        auto& context = GetGraphicContext();
        auto& cmd = context.GetCurrentCommandBuffer();
        auto& window = Application::Get().GetWindow();

        uint32_t imageIndex = context.GetActiveImageIndex();
        auto imageUsage = context.GetSwapchainImageUsage(imageIndex);
        auto image = context.AcquireImage(imageIndex, ImageUsage::eColorAttachment);
        cmd->ImageBarrier(image, imageUsage, ImageUsage::eColorAttachment);
        cmd->BeginRenderPass(mRenderPass, context.GetDefaultFramebuffer(imageIndex));
        cmd->SetViewport(window->GetWidth(), window->GetHeight(), 0.0f, 1.0f, 0, 0);
        cmd->SetScissor(window->GetWidth(), window->GetHeight(), 0, 0);
        mUIContext->BeginFrame();
        static bool dockspaceOpen = true;
        static bool optFullscreenPersistent = true;
        bool opt_fullscreen = optFullscreenPersistent;
        static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
        ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);
        }

        style.WindowMinSize.x = minWinSizeX;
        ImGui::End();
        ImGui::ShowDemoWindow();
        mUIContext->EndFrame();
        cmd->EndRenderPass();
    }
}
