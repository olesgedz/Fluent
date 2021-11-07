#include <filesystem>
#include <tinyimageformat_base.h>
#include <tiny_ktx.h>
#include <fstream>
#include "Core/FileSystem.hpp"
#include "Renderer/DeviceAllocator.hpp"
#include "Renderer/GraphicContext.hpp"
#include "Renderer/Image.hpp"

namespace Fluent
{
    static std::vector<char> LoadKtxImageDescription(ImageDescription& description)
    {
        TinyKtx_Callbacks callbacks
        {
            [](void* user, char const* msg) { LOG_ERROR("KTX Image load failed {}", msg); },
            [](void* user, size_t size) { return malloc(size); },
            [](void* user, void* memory) { free(memory); },
            [](void* user, void* buffer, size_t byteCount) 
            { 
                std::ifstream& ifs = *((std::ifstream*)user);
                ifs.read(static_cast<char*>(buffer), byteCount);
                return byteCount;
            },
            [](void* user, int64_t offset) 
            { 
                std::ifstream& ifs = *((std::ifstream*)user);
                ifs.seekg(offset);
                return true;
            },
            [](void *user) 
            { 
                std::ifstream& ifs = *((std::ifstream*)user);
                return (int64_t)ifs.tellg();
            }
        };
        
        std::ifstream ifs(FileSystem::GetTexturesDirectory() + description.filename, std::ios::binary);
        TinyKtx_ContextHandle ctx = TinyKtx_CreateContext(&callbacks, &ifs);
        bool headerOkay = TinyKtx_ReadHeader(ctx);
        if (!headerOkay)
        {
            TinyKtx_DestroyContext(ctx);
            LOG_WARN("Failed to read ktx header");
        }

        description.width = TinyKtx_Width(ctx);
        description.height = TinyKtx_Height(ctx);
        description.depth = std::max(1u, TinyKtx_Depth(ctx));
        description.arraySize = std::max(1u, TinyKtx_ArraySlices(ctx));
        description.mipLevels = std::max(1u, TinyKtx_NumberOfMipmaps(ctx));
        description.format = (Format)TinyImageFormat_FromTinyKtxFormat(TinyKtx_GetFormat(ctx));
        description.descriptors = DescriptorType::eSampledImage;
        description.sampleCount = SampleCount::e1;

        if (description.format == Format::eUndefined)
        {
            TinyKtx_DestroyContext(ctx);
            LOG_WARN("Format is undefined");
        }

        if (TinyKtx_IsCubemap(ctx))
            description.arraySize *= 6;

        TinyKtx_DestroyContext(ctx);

        return std::vector<char>(std::istreambuf_iterator(ifs), std::istreambuf_iterator<char>());
    }

    class VulkanImage : public Image
    {
    private:
        Allocation              mAllocation;
        vk::Image               mHandle;
        Format                  mFormat;
        uint32_t                mWidth;
        uint32_t                mHeight;
        uint32_t                mMipLevels;
        vk::ImageView           mImageView;

        void ApplyDescription(ImageDescription& description)
        {
            mWidth = description.width;
            mHeight = description.height;
            mFormat = description.format;
            if (static_cast<bool>(description.flags & ImageDescriptionFlagBits::eGenerateMipMaps))
            {
                description.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(mWidth, mHeight)))) + 1;
            }
            mMipLevels = description.mipLevels;
        }

        void InitImage(ImageDescription description)
        {
            if (!mHandle)
            {
                auto& context = GetGraphicContext();
                auto& allocator = context.GetDeviceAllocator();

                if (!description.filename.empty())
                {
                    auto imageData = LoadKtxImageDescription(description);
                    ApplyDescription(description);
                    auto stage = context.GetStagingBuffer()->Submit(imageData.data(), imageData.size() * sizeof(imageData[0]));
                    auto [image, allocation] = allocator.AllocateImage(description, MemoryUsage::eGpu);
                    
                    // TODO: Not beautiful solution
                    // Don't forget to fill data from loaded image
                    mHandle = static_cast<VkImage>(image);
                    mAllocation = allocation;

                    auto& cmd = context.GetCurrentCommandBuffer();
                    cmd->Begin();
                    cmd->CopyBufferToImage(context.GetStagingBuffer()->GetBuffer(), stage.offset, *this, ImageUsage::eUndefined);
                    cmd->GenerateMipLevels(*this, ImageUsage::eTransferDst, Filter::eLinear);
                    if (description.initialUsage != ImageUsage::eUndefined)
                    {
                        cmd->ImageBarrier(*this, ImageUsage::eTransferDst, description.initialUsage);
                    }
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
            LOG_TRACE("[ Image Created ] Size {}x{} Format {}", mWidth, mHeight, vk::to_string(ToVulkanFormat(mFormat)));
        }
    public:
        VulkanImage(const ImageDescription& description)
            : mAllocation(nullptr)
            , mHandle((VkImage)description.handle)
            , mFormat(description.format)
            , mWidth(description.width), mHeight(description.height)
            , mImageView(nullptr)
            , mMipLevels(1)
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
            auto imageSubresourceRange = GetImageSubresourceRange(*this);

            vk::ImageViewCreateInfo imageViewCreateInfo;
            imageViewCreateInfo
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(ToVulkanFormat(mFormat))
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
            return mFormat;
        }

        Handle GetNativeHandle() const override { return mHandle; }
        Handle GetImageView() const override { return mImageView; }
        uint32_t GetWidth() const override { return mWidth; };
        uint32_t GetHeight() const override { return mHeight; };
        uint32_t GetMipLevelsCount() const override { return mMipLevels; }
    };

    Ref<Image> Image::Create(const ImageDescription& description)
    {
        return CreateRef<VulkanImage>(description);
    }
} // namespace Fluent
