#include <volk.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include "Core/Application.hpp"
#include "Renderer/GraphicContext.hpp"
#include "Renderer/RenderPass.hpp"
#include "UI/UIContext.hpp"

namespace Fluent
{
    class VulkanUI : public UIContext
    {
    private:
        VkDescriptorPool  mDescriptorPool;
    public:
        VulkanUI(const UIContextDescription& description)
        {
            auto& context = GetGraphicContext();
            ImGui::CreateContext();
            auto& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            if (description.docking)
            {
                io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            }

            if (description.viewports)
            {
                io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
            }

            ImGui_ImplVulkan_InitInfo init_info{};
            init_info.Instance              = (VkInstance)context.GetInstance();
            init_info.PhysicalDevice        = (VkPhysicalDevice)context.GetPhysicalDevice();
            init_info.Device                = (VkDevice)context.GetDevice();
            init_info.QueueFamily           = context.GetQueueIndex();
            init_info.Queue                 = (VkQueue)context.GetDeviceQueue();
            init_info.PipelineCache         = VkPipelineCache{};
            init_info.Allocator             = nullptr;
            init_info.MinImageCount         = context.GetPresentImageCount();
            init_info.ImageCount            = context.GetPresentImageCount();
            init_info.InFlyFrameCount       = context.GetPresentImageCount();
            init_info.CheckVkResultFn       = nullptr;

            VkRenderPass renderPass = (VkRenderPass)description.renderPass->GetNativeHandle();

            ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)Application::Get().GetWindow()->GetNativeHandle(), false);
            ImGui_ImplVulkan_Init(&init_info, renderPass);

            auto& cmd = context.GetCurrentCommandBuffer();
            cmd->Begin();
            ImGui_ImplVulkan_CreateFontsTexture((VkCommandBuffer)cmd->GetNativeHandle());
            cmd->End();
            context.ImmediateSubmit(cmd);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }

        ~VulkanUI() override
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            VkDevice device = (VkDevice)GetGraphicContext().GetDevice();
            vkDestroyDescriptorPool(device, mDescriptorPool, nullptr);
        }

        void BeginFrame() const override
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        void EndFrame() const override
        {
            ImGui::Render();
            auto& context = GetGraphicContext();
            auto cmd = (VkCommandBuffer)context.GetCurrentCommandBuffer()->GetNativeHandle();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
        }
    };

    Scope<UIContext> UIContext::Create(const UIContextDescription& description)
    {
        return CreateScope<VulkanUI>(description);
    }
} // namespace Fluent
