#include "Renderer/DeviceAllocator.hpp"
#include "Renderer/GraphicContext.hpp"
#include "Renderer/Image.hpp"

namespace Fluent
{
    class VulkanImage : public Image
    {
    private:
        Allocation              mAllocation;
        vk::Image               mHandle;
        vk::Format              mFormat;
        uint32_t                mWidth;
        uint32_t                mHeight;
        vk::ImageView           mImageView;
    public:
        VulkanImage(const ImageDescription& description)
            : mAllocation(nullptr)
            , mHandle((VkImage)description.handle)
            , mFormat(ToVulkanFormat(description.format))
            , mWidth(description.width), mHeight(description.height)
            , mImageView(nullptr)
        {
            if (!mHandle)
            {
                auto& allocator = GetGraphicContext().GetDeviceAllocator();
                auto [image, allocation] = allocator.AllocateImage(description, MemoryUsage::eGpu);
                mHandle = static_cast<VkImage>(image);
                mAllocation = allocation;
                CreateImageView();
            }

            LOG_TRACE("[ Image Created ] Size {}x{} Format {}", mWidth, mHeight, vk::to_string(mFormat));
        }
        
        ~VulkanImage() override
        {
            if (mAllocation)
            {
                auto& allocator = GetGraphicContext().GetDeviceAllocator();
                allocator.FreeImage(mHandle, mAllocation);
                mAllocation = nullptr;

                if (mImageView)
                {
                    vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
                    device.destroyImageView(mImageView);
                }
            }
        }

        void CreateImageView()
        {
            /// TODO: Binding func
            vk::ImageSubresourceRange imageSubresourceRange;
            imageSubresourceRange
                .setAspectMask(ImageFormatToImageAspect(mFormat))
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            vk::ImageViewCreateInfo imageViewCreateInfo;
            imageViewCreateInfo
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(static_cast<vk::Format>(mFormat))
                .setImage(mHandle)
                .setSubresourceRange(imageSubresourceRange)
                .setComponents(vk::ComponentMapping{
                        vk::ComponentSwizzle::eIdentity,
                        vk::ComponentSwizzle::eIdentity,
                        vk::ComponentSwizzle::eIdentity,
                        vk::ComponentSwizzle::eIdentity
                });

            vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
            mImageView = device.createImageView(imageViewCreateInfo);
        }

        Format GetFormat() const override
        {
            return FromVulkanFormatToFormat(mFormat);
        }

        Handle GetNativeHandle() const override { return mHandle; }
        Handle GetImageView() const override { return mImageView; }
        uint32_t GetWidth() const override { return mWidth; };
        uint32_t GetHeight() const override { return mHeight; };
    };

    Ref<Image> Image::Create(const ImageDescription& description)
    {
        return CreateRef<VulkanImage>(description);
    }
} // namespace Fluent
