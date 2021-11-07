#include <vulkan/vulkan.hpp>
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
        vk::DescriptorPool  mDescriptorPool;
    public:
        VulkanUI(const UIContextDescription& description)
        {
            auto& context = GetGraphicContext();
            std::vector<vk::DescriptorPoolSize> poolSizes = 
            {
                { vk::DescriptorType::eSampler, 1000 },
                { vk::DescriptorType::eCombinedImageSampler, 1000 },
                { vk::DescriptorType::eSampledImage, 1000 },
                { vk::DescriptorType::eStorageImage, 1000 },
                { vk::DescriptorType::eUniformTexelBuffer, 1000 },
                { vk::DescriptorType::eStorageTexelBuffer, 1000 },
                { vk::DescriptorType::eUniformBuffer, 1000 },
                { vk::DescriptorType::eStorageBuffer, 1000 },
                { vk::DescriptorType::eUniformBufferDynamic, 1000 },
                { vk::DescriptorType::eStorageBufferDynamic, 1000 },
                { vk::DescriptorType::eInputAttachment, 1000 }
            };

            vk::DescriptorPoolCreateInfo poolInfo;
            poolInfo
                .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
                .setMaxSets(1000)
                .setPoolSizes(poolSizes);

            vk::Device device = (VkDevice)context.GetDevice();
            mDescriptorPool = device.createDescriptorPool(poolInfo);

            ImGui::CreateContext();
            ImGui_ImplVulkan_InitInfo init_info{};
            init_info.Instance              = (VkInstance)context.GetInstance();
            init_info.PhysicalDevice        = (VkPhysicalDevice)context.GetPhysicalDevice();
            init_info.Device                = (VkDevice)context.GetDevice();
            init_info.QueueFamily           = context.GetQueueIndex();
            init_info.Queue                 = (VkQueue)context.GetDeviceQueue();
            init_info.PipelineCache         = vk::PipelineCache{};
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
            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            device.destroyDescriptorPool(mDescriptorPool);
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
