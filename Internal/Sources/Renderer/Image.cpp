#include <filesystem>
#include <stb_image.h>
#include "Core/FileSystem.hpp"
#include "Renderer/DeviceAllocator.hpp"
#include "Renderer/GraphicContext.hpp"
#include "Renderer/Image.hpp"

namespace Fluent
{
    std::vector<unsigned char> GetImageData(const std::string& filename, int& width, int& height, int& texChannels)
    {
        if (!std::filesystem::exists(filename))
            LOG_WARN("File not found {}", filename);
        stbi_set_flip_vertically_on_load(true);
        stbi_uc* pixels = stbi_load(filename.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
        uint32_t size = width * height * 4;
        std::vector<unsigned char> res(pixels, pixels + size);
        stbi_image_free(pixels);
        stbi_set_flip_vertically_on_load(false);
        return res;
    }

    class VulkanImage : public Image
    {
    private:
        Allocation              mAllocation;
        vk::Image               mHandle;
        vk::Format              mFormat;
        uint32_t                mWidth;
        uint32_t                mHeight;
        vk::ImageView           mImageView;

        void InitImage(ImageDescription description)
        {
            if (!mHandle)
            {
                auto& context = GetGraphicContext();
                auto& allocator = context.GetDeviceAllocator();

                if (!description.filename.empty())
                {
                    int width, height, bpp;
                    auto imageData = GetImageData(FileSystem::GetTexturesDirectory() + description.filename, width, height, bpp);
                    description.width = width;
                    description.height = height;
                    auto stage = context.GetStagingBuffer()->Submit(imageData.data(), imageData.size() * sizeof(imageData[0]));
                    auto [image, allocation] = allocator.AllocateImage(description, MemoryUsage::eGpu);

                    // Not forget to fill data from loaded image
                    mHandle = (VkImage)image;
                    mAllocation = allocation;
                    mWidth = description.width;
                    mHeight = description.height;

                    auto& cmd = context.GetCurrentCommandBuffer();
                    cmd->Begin();
                    cmd->CopyBufferToImage(context.GetStagingBuffer()->GetBuffer(), stage.offset, *this, ImageUsage::eUndefined);
                    cmd->End();
                    context.ImmediateSubmit(cmd);
                }
                else
                {
                    auto [image, allocation] = allocator.AllocateImage(description, MemoryUsage::eGpu);
                    mHandle = static_cast<VkImage>(image);
                    mAllocation = allocation;
                }
            }

            CreateImageView();
            LOG_TRACE("[ Image Created ] Size {}x{} Format {}", mWidth, mHeight, vk::to_string(mFormat));
        }
    public:
        VulkanImage(const ImageDescription& description)
            : mAllocation(nullptr)
            , mHandle((VkImage)description.handle)
            , mFormat(ToVulkanFormat(description.format))
            , mWidth(description.width), mHeight(description.height)
            , mImageView(nullptr)
        {
            InitImage(description);
        }
        
        ~VulkanImage() override
        {
            if (mImageView)
            {
                vk::Device device = (VkDevice)GetGraphicContext().GetDevice();
                device.destroyImageView(mImageView);
            }
            
            if (mAllocation)
            {
                auto& allocator = GetGraphicContext().GetDeviceAllocator();
                allocator.FreeImage(mHandle, mAllocation);
                mAllocation = nullptr;
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
