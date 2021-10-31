#include <tinyimageformat_apis.h>
#include "Renderer/Image.hpp"
#include "Renderer/Renderer.hpp"

namespace Fluent
{
    vk::Format ToVulkanFormat(Format format)
    {
        return static_cast<vk::Format>(TinyImageFormat_ToVkFormat(static_cast<TinyImageFormat>(format)));
    }

    Format FromVulkanFormatToFormat(vk::Format format)
    {
        return static_cast<Format>(TinyImageFormat_FromVkFormat(static_cast<TinyImageFormat_VkFormat>(format)));
    }

    vk::ImageUsageFlagBits ToVulkanImageUsage(ImageUsage::Bits imageUsage)
    {
        switch (imageUsage)
        {
            case ImageUsage::eTransferSrc: return vk::ImageUsageFlagBits::eTransferSrc;
            case ImageUsage::eTransferDst: return vk::ImageUsageFlagBits::eTransferDst;
            case ImageUsage::eSampled: return vk::ImageUsageFlagBits::eSampled;
            case ImageUsage::eStorage: return vk::ImageUsageFlagBits::eStorage;
            case ImageUsage::eColorAttachment: return vk::ImageUsageFlagBits::eColorAttachment;
            case ImageUsage::eDepthStencilAttachment: return vk::ImageUsageFlagBits::eDepthStencilAttachment;
            case ImageUsage::eInputAttachment: return vk::ImageUsageFlagBits::eInputAttachment;
            case ImageUsage::eFragmentShadingRateAttachment: return vk::ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR;
        }

        return vk::ImageUsageFlagBits(-1);
    }

    vk::SampleCountFlagBits ToVulkanSampleCount(SampleCount sampleCount)
    {
        switch (sampleCount)
        {
            case SampleCount::e1: return vk::SampleCountFlagBits::e1;
            case SampleCount::e2: return vk::SampleCountFlagBits::e2;
            case SampleCount::e4: return vk::SampleCountFlagBits::e4;
            case SampleCount::e8: return vk::SampleCountFlagBits::e8;
            case SampleCount::e16: return vk::SampleCountFlagBits::e16;
            case SampleCount::e32: return vk::SampleCountFlagBits::e32;
        }

        return vk::SampleCountFlagBits(-1);
    }

    vk::BufferUsageFlagBits ToVulkanBufferUsage(BufferUsage::Bits bufferUsage)
    {
        switch (bufferUsage)
        {
            case BufferUsage::eTransferSrc: return vk::BufferUsageFlagBits::eTransferSrc;
            case BufferUsage::eTransferDst: return vk::BufferUsageFlagBits::eTransferDst;
            case BufferUsage::eUniformTexelBuffer: return vk::BufferUsageFlagBits::eUniformTexelBuffer;
            case BufferUsage::eStorageTexelBuffer: return vk::BufferUsageFlagBits::eStorageTexelBuffer;
            case BufferUsage::eUniformBuffer: return vk::BufferUsageFlagBits::eUniformBuffer;
            case BufferUsage::eStorageBuffer: return vk::BufferUsageFlagBits::eStorageBuffer;
            case BufferUsage::eIndexBuffer: return vk::BufferUsageFlagBits::eIndexBuffer;
            case BufferUsage::eVertexBuffer: return vk::BufferUsageFlagBits::eVertexBuffer;
            case BufferUsage::eIndirectBuffer: return vk::BufferUsageFlagBits::eIndirectBuffer;
            case BufferUsage::eShaderDeviceAddress: return vk::BufferUsageFlagBits::eShaderDeviceAddress;
        }

        return vk::BufferUsageFlagBits(-1);
    }

    vk::AttachmentLoadOp ToVulkanLoadOp(AttachmentLoadOp loadOp)
    {
        switch (loadOp)
        {
            case AttachmentLoadOp::eClear: return vk::AttachmentLoadOp::eClear;
            case AttachmentLoadOp::eDontCare: return vk::AttachmentLoadOp::eDontCare;
            case AttachmentLoadOp::eLoad: return vk::AttachmentLoadOp::eLoad;
        }
        return vk::AttachmentLoadOp(-1);
    }

    vk::ShaderStageFlagBits ToVulkanShaderStage(ShaderStage shaderStage)
    {
        switch (shaderStage)
        {
            case ShaderStage::eVertex: return vk::ShaderStageFlagBits::eVertex;
            case ShaderStage::eTessellationControl: return vk::ShaderStageFlagBits::eTessellationControl;
            case ShaderStage::eTessellationEvaluation: return vk::ShaderStageFlagBits::eTessellationEvaluation;
            case ShaderStage::eGeometry: return vk::ShaderStageFlagBits::eGeometry;
            case ShaderStage::eFragment: return vk::ShaderStageFlagBits::eFragment;
            case ShaderStage::eCompute: return vk::ShaderStageFlagBits::eCompute;
            case ShaderStage::eAllGraphics: return vk::ShaderStageFlagBits::eAllGraphics;
            case ShaderStage::eAll: return vk::ShaderStageFlagBits::eAll;
            case ShaderStage::eRaygenKHR: return vk::ShaderStageFlagBits::eRaygenKHR;
            case ShaderStage::eAnyHitKHR: return vk::ShaderStageFlagBits::eAnyHitKHR;
            case ShaderStage::eClosestHitKHR: return vk::ShaderStageFlagBits::eClosestHitKHR;
            case ShaderStage::eMissKHR: return vk::ShaderStageFlagBits::eMissKHR;
            case ShaderStage::eIntersectionKHR: return vk::ShaderStageFlagBits::eIntersectionKHR;
            case ShaderStage::eCallableKHR: return vk::ShaderStageFlagBits::eCallableKHR;
        }
        return vk::ShaderStageFlagBits(-1);
    }

    vk::CullModeFlagBits ToVulkanCullMode(CullMode cullMode)
    {
        switch (cullMode)
        {
            case CullMode::eBack: return vk::CullModeFlagBits::eBack;
            case CullMode::eFront: return vk::CullModeFlagBits::eFront;
            case CullMode::eNone: return vk::CullModeFlagBits::eNone;
        }

        return vk::CullModeFlagBits(-1);
    }

    vk::FrontFace ToVulkanFrontFace(FrontFace frontFace)
    {
        switch(frontFace)
        {
            case FrontFace::eClockwise: return vk::FrontFace::eClockwise;
            case FrontFace::eCounterClockwise: return vk::FrontFace::eCounterClockwise;
        }

        return vk::FrontFace(-1);
    }

    vk::Filter ToVulkanFilter(Filter filter)
    {
        switch (filter)
        {
            case Filter::eLinear: return vk::Filter::eLinear;
            case Filter::eNearest: return vk::Filter::eNearest;
        }
        
        return vk::Filter(-1);
    }

    vk::PipelineBindPoint ToVulkanPipelineBindPoint(PipelineType type)
    {
        switch (type)
        {
            case PipelineType::eCompute: return vk::PipelineBindPoint::eCompute;
            case PipelineType::eGraphics: return vk::PipelineBindPoint::eGraphics;
            case PipelineType::eRayTracing: return vk::PipelineBindPoint::eRayTracingKHR;
        }

        return vk::PipelineBindPoint(-1);
    }

    vk::VertexInputRate ToVulkanVertexInputRate(VertexInputRate inputRate)
    {
        switch (inputRate)
        {
            case VertexInputRate::eVertex: return vk::VertexInputRate::eVertex;
            case VertexInputRate::eInstance: return vk::VertexInputRate::eInstance;
        }
        return vk::VertexInputRate(-1);
    }

    vk::DescriptorType ToVulkanDescriptorType(DescriptorType type)
    {
        switch(type)
        {
            case DescriptorType::eSampler: return vk::DescriptorType::eSampler;
            case DescriptorType::eCombinedImageSampler: return vk::DescriptorType::eCombinedImageSampler;
            case DescriptorType::eSampledImage: return vk::DescriptorType::eSampledImage;
            case DescriptorType::eStorageImage: return vk::DescriptorType::eStorageImage;
            case DescriptorType::eUniformTexelBuffer: return vk::DescriptorType::eUniformTexelBuffer;
            case DescriptorType::eStorageTexelBuffer: return vk::DescriptorType::eStorageTexelBuffer;
            case DescriptorType::eUniformBuffer: return vk::DescriptorType::eUniformBuffer;
            case DescriptorType::eStorageBuffer: return vk::DescriptorType::eStorageBuffer;
            case DescriptorType::eUniformBufferDynamic: return vk::DescriptorType::eUniformBufferDynamic;
            case DescriptorType::eStorageBufferDynamic: return vk::DescriptorType::eStorageBufferDynamic;
            case DescriptorType::eInputAttachment: return vk::DescriptorType::eInputAttachment;
        }

        return vk::DescriptorType(-1);
    }
    
    vk::IndexType ToVulkanIndexType(IndexType type)
    {
        switch (type)
        {
            case IndexType::eUint16: return vk::IndexType::eUint16;
            case IndexType::eUint32: return vk::IndexType::eUint32;
        }
        
        return vk::IndexType(-1);
    }

    vk::ImageAspectFlags ImageFormatToImageAspect(vk::Format format)
    {
        static const std::unordered_map<vk::Format, vk::ImageAspectFlags> formatToAspect
        {
            { vk::Format::eD16Unorm, vk::ImageAspectFlagBits::eDepth },
            { vk::Format::eX8D24UnormPack32, vk::ImageAspectFlagBits::eDepth },
            { vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth },
            { vk::Format::eD16UnormS8Uint, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil },
            { vk::Format::eD24UnormS8Uint, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil },
            { vk::Format::eD32SfloatS8Uint, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil }
        };

        if (formatToAspect.find(format) != formatToAspect.cend())
            return formatToAspect.at(format);
        else
            return vk::ImageAspectFlagBits::eColor;
    }

    vk::AccessFlags ImageUsageToAccessFlags(ImageUsage::Bits usage)
    {
        static const std::unordered_map<ImageUsage::Bits, vk::AccessFlags> usageToAccess
        {
            { ImageUsage::eUndefined, vk::AccessFlags{} },
            { ImageUsage::eTransferSrc, vk::AccessFlagBits::eTransferRead },
            { ImageUsage::eTransferDst, vk::AccessFlagBits::eTransferWrite },
            { ImageUsage::eSampled, vk::AccessFlagBits::eShaderRead },
            { ImageUsage::eStorage, vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite },
            { ImageUsage::eColorAttachment, vk::AccessFlagBits::eColorAttachmentWrite },
            { ImageUsage::eDepthStencilAttachment, vk::AccessFlagBits::eDepthStencilAttachmentWrite },
            { ImageUsage::eInputAttachment, vk::AccessFlagBits::eInputAttachmentRead },
            { ImageUsage::eFragmentShadingRateAttachment, vk::AccessFlagBits::eFragmentShadingRateAttachmentReadKHR }
        };

        if (usageToAccess.find(usage) != usageToAccess.cend())
            return usageToAccess.at(usage);
        else
            return vk::AccessFlags{};
    }

    vk::ImageLayout ImageUsageToImageLayout(ImageUsage::Bits usage)
    {
        static const std::unordered_map<ImageUsage::Bits, vk::ImageLayout> usageToLayout
        {
            { ImageUsage::eUndefined, vk::ImageLayout::eUndefined },
            { ImageUsage::eTransferSrc, vk::ImageLayout::eTransferSrcOptimal },
            { ImageUsage::eTransferDst, vk::ImageLayout::eTransferDstOptimal },
            { ImageUsage::eSampled, vk::ImageLayout::eShaderReadOnlyOptimal },
            { ImageUsage::eStorage, vk::ImageLayout::eGeneral },
            { ImageUsage::eColorAttachment, vk::ImageLayout::eColorAttachmentOptimal },
            { ImageUsage::eDepthStencilAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal },
            { ImageUsage::eInputAttachment, vk::ImageLayout::eAttachmentOptimalKHR },
            { ImageUsage::eFragmentShadingRateAttachment, vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR }
        };

        if (usageToLayout.find(usage) != usageToLayout.cend())
            return usageToLayout.at(usage);
        else
            return vk::ImageLayout::eUndefined;
    }

    vk::PipelineStageFlags ImageUsageToPipelineStage(ImageUsage::Bits usage)
    {
        static const std::unordered_map<ImageUsage::Bits, vk::PipelineStageFlags> usageToStage
        {
            { ImageUsage::eUndefined, vk::PipelineStageFlagBits::eTopOfPipe },
            { ImageUsage::eTransferSrc, vk::PipelineStageFlagBits::eTransfer },
            { ImageUsage::eTransferDst, vk::PipelineStageFlagBits::eTransfer },
            { ImageUsage::eSampled, vk::PipelineStageFlagBits::eFragmentShader },
            { ImageUsage::eStorage, vk::PipelineStageFlagBits::eFragmentShader },
            { ImageUsage::eColorAttachment, vk::PipelineStageFlagBits::eColorAttachmentOutput },
            { ImageUsage::eDepthStencilAttachment, vk::PipelineStageFlagBits::eEarlyFragmentTests },
            { ImageUsage::eInputAttachment, vk::PipelineStageFlagBits::eFragmentShader },
            { ImageUsage::eFragmentShadingRateAttachment, vk::PipelineStageFlagBits::eFragmentShadingRateAttachmentKHR }
        };

        if (usageToStage.find(usage) != usageToStage.cend())
            return usageToStage.at(usage);
        else
            return vk::PipelineStageFlagBits::eTopOfPipe;
    }

    vk::ImageSubresourceRange GetImageSubresourceRange(const Ref<Image>& image)
    {
        vk::ImageSubresourceRange imageSubresourceRange;
        imageSubresourceRange
            .setAspectMask(ImageFormatToImageAspect(static_cast<vk::Format>(image->GetFormat())))
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);
        return imageSubresourceRange;
    }

    vk::ImageSubresourceLayers GetImageSubresourceLayers(const Ref<Image>& image)
    {
        auto subresourceRange = GetImageSubresourceRange(image);
        return vk::ImageSubresourceLayers{
            subresourceRange.aspectMask,
            subresourceRange.baseMipLevel,
            subresourceRange.baseArrayLayer,
            subresourceRange.layerCount
        };
    }
} // namespace Fluent
