#include <vulkan/vulkan.h>
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
            std::vector<VkDescriptorPoolSize> poolSizes =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            poolInfo.maxSets = 1000;
            poolInfo.poolSizeCount = poolSizes.size();
            poolInfo.pPoolSizes = poolSizes.data();

            VkDevice device = (VkDevice)context.GetDevice();
            VK_ASSERT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &mDescriptorPool));

            ImGui::CreateContext();
            ImGui_ImplVulkan_InitInfo init_info{};
            init_info.Instance              = (VkInstance)context.GetInstance();
            init_info.PhysicalDevice        = (VkPhysicalDevice)context.GetPhysicalDevice();
            init_info.Device                = (VkDevice)context.GetDevice();
            init_info.QueueFamily           = context.GetQueueIndex();
            init_info.Queue                 = (VkQueue)context.GetDeviceQueue();
            init_info.PipelineCache         = VkPipelineCache{};
            init_info.DescriptorPool        = mDescriptorPool;
            init_info.Allocator             = nullptr;
            init_info.MinImageCount         = context.GetPresentImageCount();
            init_info.ImageCount            = context.GetPresentImageCount();
            init_info.CheckVkResultFn       = nullptr;

            VkRenderPass renderPass = (VkRenderPass)description.renderPass->GetNativeHandle();
            ImGui_ImplVulkan_Init(&init_info, renderPass);

            auto& cmd = context.GetCurrentCommandBuffer();
            cmd->Begin();
            ImGui_ImplVulkan_CreateFontsTexture((VkCommandBuffer)cmd->GetNativeHandle());
            cmd->End();
            context.ImmediateSubmit(cmd);

            ImGui_ImplVulkan_DestroyFontUploadObjects();
            ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)Application::Get().GetWindow()->GetNativeHandle(), false);
        }

        ~VulkanUI() override
        {
            ImGui_ImplGlfw_Shutdown();
            ImGui_ImplVulkan_Shutdown();
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
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), (VkCommandBuffer)context.GetCurrentCommandBuffer()->GetNativeHandle());
        }
    };

    Scope<UIContext> UIContext::Create(const UIContextDescription& description)
    {
        return CreateScope<VulkanUI>(description);
    }
} // namespace Fluent
