#include <algorithm>
#include <volk.h>
#include <GLFW/glfw3.h>
#include "Renderer/VirtualFrame.hpp"
#include "Renderer/GraphicContext.hpp"

namespace Fluent
{
    /// Interface 
    static GraphicContext* sGraphicContext;
    ///
    
    std::vector<const char*> GetBestInstanceLayers(bool validationRequested)
    {
        std::vector<const char*> result;

        if (validationRequested)
            result.emplace_back("VK_LAYER_KHRONOS_validation");
        
        return result;
    };

    std::vector<const char*> GetBestInstanceExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        return { glfwExtensions, glfwExtensions + glfwExtensionCount };
    }

    class VulkanContext : public GraphicContext
    {
    private:
        void*                           mWindowHandle;
        VkInstance                      mInstance = VK_NULL_HANDLE;
        VkSurfaceKHR                    mSurface = VK_NULL_HANDLE;
        VkPhysicalDevice                mPhysicalDevice;
        VkDevice                        mDevice = VK_NULL_HANDLE;
        uint32_t                        mQueueIndex;
        VkQueue                         mDeviceQueue = VK_NULL_HANDLE;
        VkSurfaceFormatKHR              mSurfaceFormat;
        VkPresentModeKHR                mPresentMode;
        uint32_t                        mPresentImageCount;
        Scope<DeviceAllocator>          mDeviceAllocator;
        VkCommandPool                   mCommandPool = VK_NULL_HANDLE;
        uint32_t                        mActiveImageIndex{};
        bool                            mRenderingEnabled{};
        VkExtent2D                      mExtent{};
        VkSwapchainKHR                  mSwapchain = VK_NULL_HANDLE;
        std::vector<Ref<Image>>         mSwapchainImages;
        std::vector<ImageUsage::Bits>   mSwapchainImageUsages;
        VkDescriptorPool                mDescriptorPool = VK_NULL_HANDLE;

        static constexpr uint32_t       FRAME_COUNT = 2;
        Scope<VirtualFrameProvider>     mFrameProvider;

        Ref<RenderPass>                 mDefaultRenderPass;
        std::vector<Ref<Framebuffer>>   mDefaultFramebuffers;

        void CreateDescriptorPool()
        {
            // TODO
            const uint32_t poolSizeCount = 11;
            VkDescriptorPoolSize descriptorPoolSizes [poolSizeCount] =
            {
                { VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER,                1024 },
                { VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
                { VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1024 },
                { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1024 },
                { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1024 },
                { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1024 },
                { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1024 },
                { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1024 },
                { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 },
                { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024 },
                { VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1024 },
            };

            VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
            descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptorPoolCreateInfo.flags = 0;
            descriptorPoolCreateInfo.poolSizeCount = poolSizeCount;
            descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;
            descriptorPoolCreateInfo.maxSets = 2048 * poolSizeCount;

            VK_ASSERT(vkCreateDescriptorPool(mDevice, &descriptorPoolCreateInfo, nullptr, &mDescriptorPool));
        }
    public:
        explicit VulkanContext(const GraphicContextDescription& description)
            : mWindowHandle(description.window)
        {
            auto* oldContext = &GetGraphicContext();
            SetGraphicContext(*this);

            volkInitialize();

            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pEngineName = "Fluent";
            appInfo.apiVersion = FLUENT_VK_API_VERSION;

            auto instanceLayers = GetBestInstanceLayers(description.requestValidation);
            auto instanceExtensions = GetBestInstanceExtensions();

            VkInstanceCreateInfo instanceCI{};
            instanceCI.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instanceCI.pApplicationInfo         = &appInfo;
            instanceCI.enabledExtensionCount    = static_cast<uint32_t>(instanceExtensions.size());
            instanceCI.ppEnabledExtensionNames  = instanceExtensions.data();
            instanceCI.enabledLayerCount        = static_cast<uint32_t>(instanceLayers.size());
            instanceCI.ppEnabledLayerNames      = instanceLayers.data();

            VK_ASSERT(vkCreateInstance(&instanceCI, nullptr, &mInstance));
            volkLoadInstance(mInstance);

            // TODO: Wrap
            auto result = glfwCreateWindowSurface
            (
                static_cast<VkInstance>(mInstance), 
                static_cast<GLFWwindow*>(mWindowHandle), 
                nullptr, 
                (VkSurfaceKHR*)&mSurface
            );

            /// Select physical device
            uint32_t physicalDevicesCount = 0;
            vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, nullptr);
            std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
            vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, physicalDevices.data());

            mPhysicalDevice = physicalDevices[0];
            for (const auto& physicalDevice : physicalDevices)
            {
                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(physicalDevice, &properties);

                if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    mPhysicalDevice = physicalDevice;
                    break;
                }
            }

            uint32_t queuePropertiesCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queuePropertiesCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilyProperties(queuePropertiesCount);
            vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queuePropertiesCount, queueFamilyProperties.data());

            uint32_t index = 0;
            for (const auto& property : queueFamilyProperties)
            {
                VkBool32 supportSurface = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, index, mSurface, &supportSurface);

                if
                (
                    (property.queueCount > 0) &&
                    (property.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                    supportSurface
                )
                {
                    mQueueIndex = index;
                    break;
                }
                index++;
            }

            /// Collect surface present info
            uint32_t presentModeCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, nullptr);
            std::vector<VkPresentModeKHR> presentModes(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, presentModes.data());
            VkSurfaceCapabilitiesKHR surfaceCapabilities{};
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &surfaceCapabilities);
            uint32_t surfaceFormatsCount = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &surfaceFormatsCount, nullptr);
            std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatsCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &surfaceFormatsCount, surfaceFormats.data());

            /// Find best surface present mode
            mPresentMode = VkPresentModeKHR::VK_PRESENT_MODE_IMMEDIATE_KHR;
            if (std::find(presentModes.begin(), presentModes.end(), VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.end())
                mPresentMode = VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR;

            /// Determine present image count
            mPresentImageCount = std::clamp(FRAME_COUNT, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
            /// Find best surface format
            mSurfaceFormat = surfaceFormats.front();
            for (const auto& format : surfaceFormats)
            {
                if (format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM)
                    mSurfaceFormat = format;
            }
            
            /// Find device extensions
            uint32_t extensionsCount = 0;
            vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionsCount, nullptr);
            std::vector<VkExtensionProperties> installedExtensions(extensionsCount);
            vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionsCount, installedExtensions.data());

            /// If this extension available it should be included
            auto it = std::find_if(installedExtensions.begin(), installedExtensions.end(), [](auto& p)
            {
                if (std::string(p.extensionName) == "VK_KHR_portability_subset")
                    return true;
                else
                    return false;
            });

            std::vector<const char*> deviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
            if (it != installedExtensions.end())
                deviceExtensions.emplace_back("VK_KHR_portability_subset");
            deviceExtensions.emplace_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

            /// Logical device and device queue
            float queuePriorities [] = { 1.0f };
            VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
            deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            deviceQueueCreateInfo.queueCount = 1;
            deviceQueueCreateInfo.queueFamilyIndex = mQueueIndex;
            deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

            VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
            descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
            descriptorIndexingFeatures.descriptorBindingPartiallyBound = true;

            VkPhysicalDeviceMultiviewFeatures multiviewFeatures{};
            multiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
            multiviewFeatures.multiview = true;
            multiviewFeatures.pNext = &descriptorIndexingFeatures;

            VkDeviceCreateInfo deviceCreateInfo{};
            deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceCreateInfo.queueCreateInfoCount = 1;
            deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
            deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
            deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
            deviceCreateInfo.pNext = &multiviewFeatures;

            vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice);
            volkLoadDevice(mDevice);
            vkGetDeviceQueue(mDevice, mQueueIndex, 0, &mDeviceQueue);

            /// Create memory allocator
            DeviceAllocatorDescription deviceAllocatorDescription{};
            deviceAllocatorDescription.instance = mInstance;
            deviceAllocatorDescription.physicalDevice = mPhysicalDevice;
            deviceAllocatorDescription.device = mDevice;
            mDeviceAllocator = DeviceAllocator::Create(deviceAllocatorDescription);

            /// Create command pool
            VkCommandPoolCreateInfo cmdPoolCreateInfo{};
            cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            cmdPoolCreateInfo.queueFamilyIndex = mQueueIndex;

            VK_ASSERT(vkCreateCommandPool(mDevice, &cmdPoolCreateInfo, nullptr, &mCommandPool));
            CreateDescriptorPool();

            /// Create default renderpass
            ClearValue clearValue{};
            clearValue.color = Vector4(0.0, 0.0, 0.0, 1.0);
            RenderPassDescription renderPassDesc{};
            renderPassDesc.width = mExtent.width;
            renderPassDesc.height = mExtent.height;
            renderPassDesc.clearValues = { clearValue };
            renderPassDesc.colorFormats = { FromVulkanFormatToFormat(mSurfaceFormat.format) };
            renderPassDesc.initialUsages = { ImageUsage::eColorAttachment };
            renderPassDesc.finalUsages = { ImageUsage::eColorAttachment };
            renderPassDesc.attachmentLoadOps = { AttachmentLoadOp::eLoad };
            renderPassDesc.sampleCount = SampleCount::e1;

            mDefaultRenderPass = RenderPass::Create(renderPassDesc);

            SetGraphicContext(*oldContext);
        }

        ~VulkanContext() override
        {
            mDefaultFramebuffers.clear();
            mSwapchainImages.clear();
            mFrameProvider.reset(nullptr);
            mDefaultRenderPass = nullptr;
            vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
            vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
            vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
            mDeviceAllocator.reset(nullptr);
            vkDestroyDevice(mDevice, nullptr);
            vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
            vkDestroyInstance(mInstance, nullptr);
        }

        void OnResize(uint32_t width, uint32_t height) override
        {
            mRenderingEnabled = false;
            vkDeviceWaitIdle(mDevice);

            auto surfaceWidth   = static_cast<uint32_t>(width);
            auto surfaceHeight  = static_cast<uint32_t>(height);

            VkSurfaceCapabilitiesKHR surfaceCapabilities{};
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &surfaceCapabilities);
            mExtent = VkExtent2D
                {
                    std::clamp(surfaceWidth, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
                    std::clamp(surfaceHeight, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height)
                };

            mDefaultRenderPass->SetRenderArea(mExtent.width, mExtent.height);

            VkSwapchainCreateInfoKHR swapchainCreateInfo{};
            swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            swapchainCreateInfo.surface = mSurface;
            swapchainCreateInfo.minImageCount = mPresentImageCount;
            swapchainCreateInfo.imageFormat = mSurfaceFormat.format;
            swapchainCreateInfo.imageColorSpace = mSurfaceFormat.colorSpace;
            swapchainCreateInfo.imageExtent = mExtent;
            swapchainCreateInfo.imageArrayLayers = 1;
            swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
            swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            swapchainCreateInfo.presentMode = mPresentMode;
            swapchainCreateInfo.clipped = true;
            swapchainCreateInfo.oldSwapchain = mSwapchain;

            auto result = vkCreateSwapchainKHR(mDevice, &swapchainCreateInfo, nullptr, &mSwapchain);

            if (swapchainCreateInfo.oldSwapchain)
                vkDestroySwapchainKHR(mDevice, swapchainCreateInfo.oldSwapchain, nullptr);

            /// Create swapchain images
            uint32_t swapchainImagesCount = 0;
            vkGetSwapchainImagesKHR(mDevice, mSwapchain, &swapchainImagesCount, nullptr);
            std::vector<VkImage> swapchainImages(swapchainImagesCount);
            vkGetSwapchainImagesKHR(mDevice, mSwapchain, &swapchainImagesCount, swapchainImages.data());

            ImageDescription swapchainImageDescription{};
            swapchainImageDescription.width = mExtent.width;
            swapchainImageDescription.height = mExtent.height;
            swapchainImageDescription.format = FromVulkanFormatToFormat(mSurfaceFormat.format);

            mSwapchainImages.clear();
            mSwapchainImages.reserve(swapchainImages.size());
            mDefaultFramebuffers.clear();
            mDefaultFramebuffers.reserve(swapchainImages.size());

            FramebufferDescription fbDescription{};
            fbDescription.width = mExtent.width;
            fbDescription.height = mExtent.height;
            fbDescription.renderPass = mDefaultRenderPass;

            for (auto image : swapchainImages)
            {
                swapchainImageDescription.handle = image;
                mSwapchainImages.emplace_back(Image::Create(swapchainImageDescription));
                fbDescription.targets = { mSwapchainImages.back() };
                mDefaultFramebuffers.emplace_back(Framebuffer::Create(fbDescription));
            }

            mSwapchainImageUsages.resize(mSwapchainImages.size(), ImageUsage::eUndefined);

            mFrameProvider = nullptr;
            
            VirtualFrameProviderDescription frameProviderDesc{};
            frameProviderDesc.device = mDevice;
            frameProviderDesc.commandPool = mCommandPool;
            frameProviderDesc.queue = mDeviceQueue;
            frameProviderDesc.swapchain = mSwapchain;
            frameProviderDesc.frameCount = FRAME_COUNT;
            frameProviderDesc.swapchainImageCount = mSwapchainImages.size();
            // TODO: Find optimal size
            frameProviderDesc.stagingBufferSize = 1024 * 1024 * 512;

            LOG_INFO("Current staging buffer size {}", frameProviderDesc.stagingBufferSize);

            mFrameProvider = VirtualFrameProvider::Create(frameProviderDesc);

            mRenderingEnabled = true;
        }

        Ref<Image> AcquireImage(uint32_t imageIndex, ImageUsage::Bits usage) override
        {
            mSwapchainImageUsages[imageIndex] = usage;
            return mSwapchainImages[imageIndex];
        }

        bool CanRender() const override
        {
            return mRenderingEnabled;
        }

        void BeginFrame() override
        {
            mRenderingEnabled = mFrameProvider->BeginFrame();
        }

        void EndFrame() override
        {
            mRenderingEnabled = mFrameProvider->EndFrame();
        }

        void WaitIdle() override
        {
            vkDeviceWaitIdle(mDevice);
        }
        
        void ImmediateSubmit(const Ref<CommandBuffer>& cmd) const override
        {
            auto nativeCmd = (VkCommandBuffer)cmd->GetNativeHandle();
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &nativeCmd;
            vkQueueSubmit(mDeviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(mDeviceQueue);
        }


        Ref<RenderPass> GetDefaultRenderPass() const override { return mDefaultRenderPass; }
        Ref<Framebuffer> GetDefaultFramebuffer(uint32_t index) const override { return mDefaultFramebuffers[index]; }

        ImageUsage::Bits GetSwapchainImageUsage(uint32_t index) const override { return mSwapchainImageUsages[index]; }

        Handle              GetInstance() const override { return mInstance; };
        Handle              GetPhysicalDevice() const override { return mPhysicalDevice; };
        Handle              GetDevice() override { return mDevice; }
        uint32_t            GetQueueIndex() override { return mQueueIndex; }
        Handle              GetDeviceQueue() override { return mDeviceQueue; };
        DeviceAllocator&    GetDeviceAllocator() override { return *mDeviceAllocator; }
        Handle              GetCommandPool() override { return mCommandPool; }
        Handle              GetSwapchain() override { return mSwapchain; }
        Handle              GetDescriptorPool() const override { return mDescriptorPool; }
        uint32_t            GetPresentImageCount() const override { return mPresentImageCount; }
        uint32_t            GetActiveImageIndex() const override { return mFrameProvider->GetActiveImageIndex(); };
        Ref<CommandBuffer>& GetCurrentCommandBuffer() override { return mFrameProvider->GetCommandBuffer(); }
        Ref<StagingBuffer>& GetStagingBuffer() override { return mFrameProvider->GetStagingBuffer(); }
    };

    /// Interface

    Scope<GraphicContext> GraphicContext::Create(const GraphicContextDescription& description)
    {
        return CreateScope<VulkanContext>(description);
    }

    void SetGraphicContext(GraphicContext& context)
    {
        sGraphicContext = std::addressof(context);
    }

    GraphicContext& GetGraphicContext()
    {
        return *sGraphicContext;
    }
} // namespace Fluent
