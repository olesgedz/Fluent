#include <tinyimageformat_apis.h>
#include "Renderer/Image.hpp"
#include "Renderer/Renderer.hpp"

namespace Fluent
{
    VkFormat ToVulkanFormat(Format format)
    {
        return static_cast<VkFormat>(TinyImageFormat_ToVkFormat(static_cast<TinyImageFormat>(format)));
    }

    Format FromVulkanFormatToFormat(VkFormat format)
    {
        return static_cast<Format>(TinyImageFormat_FromVkFormat(static_cast<TinyImageFormat_VkFormat>(format)));
    }

    VkImageUsageFlagBits ToVulkanImageUsage(ImageUsage::Bits imageUsage)
    {
        switch (imageUsage)
        {
            case ImageUsage::eTransferSrc: return VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            case ImageUsage::eTransferDst: return VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            case ImageUsage::eSampled: return VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
            case ImageUsage::eStorage: return VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT;
            case ImageUsage::eColorAttachment: return VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            case ImageUsage::eDepthStencilAttachment: return VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            case ImageUsage::eInputAttachment: return VkImageUsageFlagBits::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            case ImageUsage::eFragmentShadingRateAttachment: return VkImageUsageFlagBits::VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
            default: break;
        }

        return VkImageUsageFlagBits(-1);
    }

    VkSampleCountFlagBits ToVulkanSampleCount(SampleCount sampleCount)
    {
        switch (sampleCount)
        {
            case SampleCount::e1: return VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
            case SampleCount::e2: return VkSampleCountFlagBits::VK_SAMPLE_COUNT_2_BIT;
            case SampleCount::e4: return VkSampleCountFlagBits::VK_SAMPLE_COUNT_4_BIT;
            case SampleCount::e8: return VkSampleCountFlagBits::VK_SAMPLE_COUNT_8_BIT;
            case SampleCount::e16: return VkSampleCountFlagBits::VK_SAMPLE_COUNT_16_BIT;
            case SampleCount::e32: return VkSampleCountFlagBits::VK_SAMPLE_COUNT_32_BIT;
            default: break;
        }

        return VkSampleCountFlagBits(-1);
    }

    VkBufferUsageFlagBits ToVulkanBufferUsage(BufferUsage::Bits bufferUsage)
    {
        switch (bufferUsage)
        {
            case BufferUsage::eTransferSrc: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            case BufferUsage::eTransferDst: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            case BufferUsage::eUniformTexelBuffer: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
            case BufferUsage::eStorageTexelBuffer: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
            case BufferUsage::eUniformBuffer: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            case BufferUsage::eStorageBuffer: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            case BufferUsage::eIndexBuffer: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            case BufferUsage::eVertexBuffer: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            case BufferUsage::eIndirectBuffer: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
            case BufferUsage::eShaderDeviceAddress: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            default: break;
        }

        return VkBufferUsageFlagBits(-1);
    }

    VkAttachmentLoadOp ToVulkanLoadOp(AttachmentLoadOp loadOp)
    {
        switch (loadOp)
        {
            case AttachmentLoadOp::eClear: return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
            case AttachmentLoadOp::eDontCare: return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            case AttachmentLoadOp::eLoad: return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
            default: break;
        }
        return VkAttachmentLoadOp(-1);
    }

    VkShaderStageFlagBits ToVulkanShaderStage(ShaderStage shaderStage)
    {
        switch (shaderStage)
        {
            case ShaderStage::eVertex: return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::eTessellationControl: return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            case ShaderStage::eTessellationEvaluation: return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            case ShaderStage::eGeometry: return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
            case ShaderStage::eFragment: return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderStage::eCompute: return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
            case ShaderStage::eAllGraphics: return VkShaderStageFlagBits::VK_SHADER_STAGE_ALL_GRAPHICS;
            case ShaderStage::eAll: return VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
            case ShaderStage::eRaygenKHR: return VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            case ShaderStage::eAnyHitKHR: return VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
            case ShaderStage::eClosestHitKHR: return VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
            case ShaderStage::eMissKHR: return VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR;
            case ShaderStage::eIntersectionKHR: return VkShaderStageFlagBits::VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
            case ShaderStage::eCallableKHR: return VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR;
            default: break;
        }
        return VkShaderStageFlagBits(-1);
    }

    VkCullModeFlagBits ToVulkanCullMode(CullMode cullMode)
    {
        switch (cullMode)
        {
            case CullMode::eBack: return VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
            case CullMode::eFront: return VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT;
            case CullMode::eNone: return VkCullModeFlagBits::VK_CULL_MODE_NONE;
            default: break;
        }

        return VkCullModeFlagBits(-1);
    }

    VkFrontFace ToVulkanFrontFace(FrontFace frontFace)
    {
        switch(frontFace)
        {
            case FrontFace::eClockwise: return VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
            case FrontFace::eCounterClockwise: return VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
            default: break;
        }

        return VkFrontFace(-1);
    }

    VkFilter ToVulkanFilter(Filter filter)
    {
        switch (filter)
        {
            case Filter::eLinear: return VK_FILTER_LINEAR;
            case Filter::eNearest: return VK_FILTER_NEAREST;
            default: break;
        }
        
        return VkFilter(-1);
    }

    VkPipelineBindPoint ToVulkanPipelineBindPoint(PipelineType type)
    {
        switch (type)
        {
            case PipelineType::eCompute: return VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE;
            case PipelineType::eGraphics: return VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
            case PipelineType::eRayTracing: return VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
            default: break;
        }

        return VkPipelineBindPoint(-1);
    }

    VkVertexInputRate ToVulkanVertexInputRate(VertexInputRate inputRate)
    {
        switch (inputRate)
        {
            case VertexInputRate::eVertex: return VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
            case VertexInputRate::eInstance: return VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE;
            default: break;
        }
        return VkVertexInputRate(-1);
    }

    VkDescriptorType ToVulkanDescriptorType(DescriptorType type)
    {
        switch(type)
        {
            case DescriptorType::eSampler: return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER;
            case DescriptorType::eCombinedImageSampler: return VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            case DescriptorType::eSampledImage: return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case DescriptorType::eStorageImage: return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case DescriptorType::eUniformTexelBuffer: return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            case DescriptorType::eStorageTexelBuffer: return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            case DescriptorType::eUniformBuffer: return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case DescriptorType::eStorageBuffer: return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case DescriptorType::eUniformBufferDynamic: return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            case DescriptorType::eStorageBufferDynamic: return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            case DescriptorType::eInputAttachment: return VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            default: break;
        }

        return VkDescriptorType(-1);
    }
    
    VkIndexType ToVulkanIndexType(IndexType type)
    {
        switch (type)
        {
            case IndexType::eUint16: return VkIndexType::VK_INDEX_TYPE_UINT16;
            case IndexType::eUint32: return VkIndexType::VK_INDEX_TYPE_UINT32;
            default: break;
        }
        
        return VkIndexType(-1);
    }

    VkSamplerMipmapMode ToVulkanSamplerMipmapMode(SamplerMipmapMode mode)
    {
        switch (mode)
        {
            case SamplerMipmapMode::eNearest: return VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST;
            case SamplerMipmapMode::eLinear: return VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
            default: break;
        }

        return VkSamplerMipmapMode(-1);
    }

    VkSamplerAddressMode ToVulkanSamplerAddressMode(SamplerAddressMode mode)
    {
        switch (mode)
        {
            case SamplerAddressMode::eRepeat: return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case SamplerAddressMode::eMirroredRepeat: return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case SamplerAddressMode::eClampToEdge: return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case SamplerAddressMode::eClampToBorder: return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            case SamplerAddressMode::eMirrorClampToEdge: return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
            default: break;
        }
        return VkSamplerAddressMode(-1);
    }

    VkCompareOp ToVulkanCompareOp(CompareOp op)
    {
        switch (op)
        {
            case CompareOp::eNever          : return VkCompareOp::VK_COMPARE_OP_NEVER;
            case CompareOp::eLess           : return VkCompareOp::VK_COMPARE_OP_LESS;
            case CompareOp::eEqual          : return VkCompareOp::VK_COMPARE_OP_EQUAL;
            case CompareOp::eLessOrEqual    : return VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
            case CompareOp::eGreater        : return VkCompareOp::VK_COMPARE_OP_GREATER;
            case CompareOp::eNotEqual       : return VkCompareOp::VK_COMPARE_OP_NOT_EQUAL;
            case CompareOp::eGreaterOrEqual : return VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL;
            case CompareOp::eAlways         : return VkCompareOp::VK_COMPARE_OP_ALWAYS;
            default: break;
        }

        return VkCompareOp(-1);
    }

    VkImageAspectFlags ImageFormatToImageAspect(VkFormat format)
    {
        static const std::unordered_map<VkFormat, VkImageAspectFlags> formatToAspect
        {
            { VkFormat::VK_FORMAT_D16_UNORM, VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT },
            { VkFormat::VK_FORMAT_X8_D24_UNORM_PACK32, VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT },
            { VkFormat::VK_FORMAT_D32_SFLOAT, VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT },
            { VkFormat::VK_FORMAT_D16_UNORM_S8_UINT, VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT | VkImageAspectFlagBits::VK_IMAGE_ASPECT_STENCIL_BIT },
            { VkFormat::VK_FORMAT_D24_UNORM_S8_UINT, VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT | VkImageAspectFlagBits::VK_IMAGE_ASPECT_STENCIL_BIT },
            { VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT, VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT | VkImageAspectFlagBits::VK_IMAGE_ASPECT_STENCIL_BIT }
        };

        if (formatToAspect.find(format) != formatToAspect.cend())
            return formatToAspect.at(format);
        else
            return VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkAccessFlags ImageUsageToAccessFlags(ImageUsage::Bits usage)
    {
        static const std::unordered_map<ImageUsage::Bits, VkAccessFlags> usageToAccess
        {
            { ImageUsage::eUndefined, VkAccessFlags{} },
            { ImageUsage::eTransferSrc, VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT },
            { ImageUsage::eTransferDst, VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT },
            { ImageUsage::eSampled, VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT },
            { ImageUsage::eStorage, VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT | VkAccessFlagBits::VK_ACCESS_SHADER_WRITE_BIT },
            { ImageUsage::eColorAttachment, VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT },
            { ImageUsage::eDepthStencilAttachment, VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT },
            { ImageUsage::eInputAttachment, VkAccessFlagBits::VK_ACCESS_INPUT_ATTACHMENT_READ_BIT },
            { ImageUsage::eFragmentShadingRateAttachment, VkAccessFlagBits::VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR }
        };

        if (usageToAccess.find(usage) != usageToAccess.cend())
            return usageToAccess.at(usage);
        else
            return VkAccessFlags{};
    }

    VkImageLayout ImageUsageToImageLayout(ImageUsage::Bits usage)
    {
        static const std::unordered_map<ImageUsage::Bits, VkImageLayout> usageToLayout
        {
            { ImageUsage::eUndefined, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED },
            { ImageUsage::eTransferSrc, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL },
            { ImageUsage::eTransferDst, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL },
            { ImageUsage::eSampled, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
            { ImageUsage::eStorage, VkImageLayout::VK_IMAGE_LAYOUT_GENERAL },
            { ImageUsage::eColorAttachment, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
            { ImageUsage::eDepthStencilAttachment, VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL },
            { ImageUsage::eInputAttachment, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
            { ImageUsage::eFragmentShadingRateAttachment, VkImageLayout::VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR }
        };

        if (usageToLayout.find(usage) != usageToLayout.cend())
            return usageToLayout.at(usage);
        else
            return VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
    }

    VkPipelineStageFlags ImageUsageToPipelineStage(ImageUsage::Bits usage)
    {
        static const std::unordered_map<ImageUsage::Bits, VkPipelineStageFlags> usageToStage
        {
            { ImageUsage::eUndefined, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT },
            { ImageUsage::eTransferSrc, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT },
            { ImageUsage::eTransferDst, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT },
            { ImageUsage::eSampled, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT },
            { ImageUsage::eStorage, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT },
            { ImageUsage::eColorAttachment, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
            { ImageUsage::eDepthStencilAttachment, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT },
            { ImageUsage::eInputAttachment, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT },
            { ImageUsage::eFragmentShadingRateAttachment, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR }
        };

        if (usageToStage.find(usage) != usageToStage.cend())
            return usageToStage.at(usage);
        else
            return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }

    VkImageSubresourceRange GetImageSubresourceRange(const Image& image)
    {
        return
        {
            ImageFormatToImageAspect(static_cast<VkFormat>(image.GetFormat())),
            0,                // base mip level
            image.GetMipLevelsCount(),   // mip levels
            0,              // base array layer
            1                  // layer count
        };
    }

    VkImageSubresourceLayers GetImageSubresourceLayers(const Image& image)
    {
        auto subresourceRange = GetImageSubresourceRange(image);
        return VkImageSubresourceLayers{
            subresourceRange.aspectMask,
            subresourceRange.baseMipLevel,
            subresourceRange.baseArrayLayer,
            subresourceRange.layerCount
        };
    }
} // namespace Fluent
