#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
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
        vk::Instance                    mInstance;
        vk::SurfaceKHR                  mSurface;
        vk::PhysicalDevice              mPhysicalDevice;
        vk::Device                      mDevice;
        uint32_t                        mQueueIndex;
        vk::Queue                       mDeviceQueue;
        vk::SurfaceFormatKHR            mSurfaceFormat;
        vk::PresentModeKHR              mPresentMode;
        uint32_t                        mPresentImageCount;
        Scope<DeviceAllocator>          mDeviceAllocator;
        vk::CommandPool                 mCommandPool;
        uint32_t                        mActiveImageIndex;
        bool                            mRenderingEnabled;
        vk::Extent2D                    mExtent;
        vk::SwapchainKHR                mSwapchain;
        std::vector<Ref<Image>>         mSwapchainImages;
        std::vector<ImageUsage::Bits>   mSwapchainImageUsages;

        static constexpr uint32_t       FRAME_COUNT = 2;
        Scope<VirtualFrameProvider>     mFrameProvider;

        void CreateSurface();
    public:
        VulkanContext(const GContextDescription& description)
            : mWindowHandle(description.window)
        {
            vk::ApplicationInfo appInfo;
            appInfo
                .setPEngineName("FluentGFX")
                .setApiVersion(VK_API_VERSION_1_2);

            auto instanceLayers = GetBestInstanceLayers(description.requestValidation);
            auto instanceExtensions = GetBestInstanceExtensions();

            vk::InstanceCreateInfo instanceCI;
            instanceCI
                .setPApplicationInfo(&appInfo)
                .setEnabledExtensionCount(static_cast<uint32_t>(instanceExtensions.size()))
                .setPpEnabledExtensionNames(instanceExtensions.data())
                .setEnabledLayerCount(static_cast<uint32_t>(instanceLayers.size()))
                .setPpEnabledLayerNames(instanceLayers.data());

            mInstance = vk::createInstance(instanceCI);

            auto result = glfwCreateWindowSurface
            (
                static_cast<VkInstance>(mInstance), 
                static_cast<GLFWwindow*>(mWindowHandle), 
                nullptr, 
                (VkSurfaceKHR*)&mSurface
            );

            /// Select physical device
            auto physicalDevices = mInstance.enumeratePhysicalDevices();
            for (const auto& physicalDevice : physicalDevices)
            {
                auto properties = physicalDevice.getProperties();
                if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                {
                    mPhysicalDevice = physicalDevice;
                    break;
                }
            }

            auto queueFamilyProperties  = mPhysicalDevice.getQueueFamilyProperties();
            uint32_t index = 0;
            for (const auto& property : queueFamilyProperties)
            {
                if ((property.queueCount > 0) &&
                    (property.queueFlags & vk::QueueFlagBits::eGraphics) &&
                    mPhysicalDevice.getSurfaceSupportKHR(index, mSurface))
                {
                    mQueueIndex = index;
                    break;
                }
                index++;
            }

            /// Collect surface present info
            auto presentModes           = mPhysicalDevice.getSurfacePresentModesKHR(mSurface);
            auto surfaceCapabilities    = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);
            auto surfaceFormats         = mPhysicalDevice.getSurfaceFormatsKHR(mSurface);

            /// Find best surface present mode
            vk::PresentModeKHR presentMode = vk::PresentModeKHR::eImmediate;
            if (std::find(presentModes.begin(), presentModes.end(), vk::PresentModeKHR::eMailbox) != presentModes.end())
                mPresentMode = vk::PresentModeKHR::eMailbox;

            /// Determine present image count
            mPresentImageCount = std::max(surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

            /// Find best surface format
            auto surfaceFormat = surfaceFormats.front();
            for (const auto& format : surfaceFormats)
            {
                if (format.format == vk::Format::eR8G8B8A8Unorm || format.format == vk::Format::eB8G8R8A8Unorm)
                    mSurfaceFormat = format;
            }
            
            /// Find device extensions
            auto installedExtensions = mPhysicalDevice.enumerateDeviceExtensionProperties();

            /// If this extension available it should be included
            auto it = std::find_if(installedExtensions.begin(), installedExtensions.end(), [](auto& p)
            {
                if (std::string(p.extensionName.data()) == "VK_KHR_portability_subset")
                    return true;
                else
                    return false;
            });

            std::vector<const char*> deviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
            if (it != installedExtensions.end())
                deviceExtensions.emplace_back("VK_KHR_portability_subset");

            /// Logical device and device queue
            std::array queuePriorities { 1.0f };
            vk::DeviceQueueCreateInfo deviceQueueCreateInfo;
            deviceQueueCreateInfo
                .setQueueFamilyIndex(mQueueIndex)
                .setQueuePriorities(queuePriorities);

            vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures;
            descriptorIndexingFeatures.descriptorBindingPartiallyBound = true;

            vk::DeviceCreateInfo deviceCreateInfo;
            deviceCreateInfo
                .setQueueCreateInfos(deviceQueueCreateInfo)
                .setPEnabledExtensionNames(deviceExtensions)
                .setPNext(&descriptorIndexingFeatures);

            mDevice = mPhysicalDevice.createDevice(deviceCreateInfo);
            mDeviceQueue  = mDevice.getQueue(mQueueIndex, 0);

            /// Create memory allocator
            DeviceAllocatorDescription deviceAllocatorDescription{};
            deviceAllocatorDescription.instance = mInstance;
            deviceAllocatorDescription.physicalDevice = mPhysicalDevice;
            deviceAllocatorDescription.device = mDevice;
            mDeviceAllocator = DeviceAllocator::Create(deviceAllocatorDescription);

            /// Create command pool
            vk::CommandPoolCreateInfo cmdPoolCreateInfo{};
            cmdPoolCreateInfo
                .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient)
                .setQueueFamilyIndex(mQueueIndex);

            mCommandPool = mDevice.createCommandPool(cmdPoolCreateInfo);

            OnResize(mExtent.width, mExtent.height);
        }

        ~VulkanContext()
        {
            mDevice.destroyCommandPool(mCommandPool);
            mDeviceAllocator = nullptr;
            mDevice.destroy();
            mInstance.destroySurfaceKHR(mSurface);
            mInstance.destroy();
        }

        void OnResize(uint32_t width, uint32_t height)
        {
            mRenderingEnabled = false;
            mDevice.waitIdle();

            auto surfaceWidth   = static_cast<uint32_t>(width);
            auto surfaceHeight  = static_cast<uint32_t>(height);

            auto surfaceCapabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);
            mExtent = vk::Extent2D(
                std::clamp(surfaceWidth,  surfaceCapabilities.minImageExtent.width,  surfaceCapabilities.maxImageExtent.width),
                std::clamp(surfaceHeight, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height)
            );

            vk::SwapchainCreateInfoKHR swapchainCreateInfo;
            swapchainCreateInfo
                .setSurface(mSurface)
                .setMinImageCount(mPresentImageCount)
                .setImageFormat(mSurfaceFormat.format)
                .setImageColorSpace(mSurfaceFormat.colorSpace)
                .setImageExtent(mExtent)
                .setImageArrayLayers(1)
                .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst)
                .setImageSharingMode(vk::SharingMode::eExclusive)
                .setPreTransform(surfaceCapabilities.currentTransform)
                .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                .setPresentMode(mPresentMode)
                .setClipped(true)
                .setOldSwapchain(mSwapchain);

            mSwapchain = mDevice.createSwapchainKHR(swapchainCreateInfo);

            if (static_cast<bool>(swapchainCreateInfo.oldSwapchain))
                mDevice.destroySwapchainKHR(swapchainCreateInfo.oldSwapchain);

            /// Create swapchain images
            const auto& swapchainImages = mDevice.getSwapchainImagesKHR(mSwapchain);
            ImageDescription swapchainImageDescription{};
            swapchainImageDescription.width = mExtent.width;
            swapchainImageDescription.height = mExtent.height;
            swapchainImageDescription.format = FromVulkanFormatToFormat(mSurfaceFormat.format);

            mSwapchainImages.clear();
            mSwapchainImages.reserve(swapchainImages.size());

            for (auto image : swapchainImages)
            {
                swapchainImageDescription.handle = image;
                mSwapchainImages.emplace_back(Image::Create(swapchainImageDescription));
            }

            mPresentImageCount = static_cast<uint32_t>(mSwapchainImages.size());
            mSwapchainImageUsages.resize(mSwapchainImages.size(), ImageUsage::eUndefined);

            mFrameProvider = nullptr;
            
            VirtualFrameProviderDescription frameProviderDesc{};
            frameProviderDesc.device = mDevice;
            frameProviderDesc.commandPool = mCommandPool;
            frameProviderDesc.queue = mDeviceQueue;
            frameProviderDesc.swapchain = mSwapchain;
            frameProviderDesc.frameCount = FRAME_COUNT;
            frameProviderDesc.swapchainImageCount = mSwapchainImages.size();

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
            mFrameProvider->BeginFrame();
        }

        void EndFrame() override
        {
            mFrameProvider->EndFrame();
        }

        void WaitIdle() override
        {
            mDevice.waitIdle();
        }
        
        ImageUsage::Bits GetSwapchainImageUsage(uint32_t index) const override { return mSwapchainImageUsages[index]; }

        Handle GetDevice() override { return mDevice; }
        DeviceAllocator& GetDeviceAllocator() override { return *mDeviceAllocator; }
        Handle GetCommandPool() override { return mCommandPool; }
        Handle GetSwapchain() override { return mSwapchain; }
        uint32_t GetActiveImageIndex() const override { return mFrameProvider->GetActiveImageIndex(); };
        Ref<CommandBuffer>& GetCurrentCommandBuffer() override { return mFrameProvider->GetCommandBuffer(); }
    };

    /// Interface

    Scope<GraphicContext> GraphicContext::Create(const GContextDescription& description)
    {
        return CreateScope<VulkanContext>(description);
    }

    void SetGraphicContext(Scope<GraphicContext>& context)
    {
        sGraphicContext = context.get();
    }

    GraphicContext& GetGraphicContext()
    {
        return *sGraphicContext;
    }
} // namespace Fluent
