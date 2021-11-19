#include <spirv_cross/spirv_glsl.hpp>
#include "Core/Base.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/ShaderReflection.hpp"

namespace Fluent
{
    ShaderType GetTypeByReflection(spirv_cross::Compiler& compiler, const spirv_cross::Resource &resource)
    {
        Format format = Format::eUndefined;
        auto& type = compiler.get_type(resource.base_type_id);
        if (type.basetype == spirv_cross::SPIRType::Float)
        {
            if (type.vecsize <  2 || type.columns <  2)
                format = Format::eR32Sfloat;
            if (type.vecsize == 2 || type.columns == 2)
                format = Format::eR32G32Sfloat;
            if (type.vecsize == 3 || type.columns == 3)
                format = Format::eR32G32B32Sfloat;
            if (type.vecsize == 4 || type.columns == 4)
                format = Format::eR32G32B32A32Sfloat;
        }

        if (type.basetype == spirv_cross::SPIRType::Int)
        {
            if (type.vecsize <  2 || type.columns <  2)
                format = Format::eR32Sint;
            if (type.vecsize == 2 || type.columns == 2)
                format =  Format::eR32G32Sint;
            if (type.vecsize == 3 || type.columns == 3)
                format = Format::eR32G32B32Sint;
            if (type.vecsize == 4 || type.columns == 4)
                format = Format::eR32G32B32A32Sint;
        }

        if (type.basetype == spirv_cross::SPIRType::UInt)
        {
            if (type.vecsize <  2 || type.columns <  2)
                format = Format::eR32Uint;
            if (type.vecsize == 2 || type.columns == 2)
                format =  Format::eR32G32Uint;
            if (type.vecsize == 3 || type.columns == 3)
                format = Format::eR32G32B32Uint;
            if (type.vecsize == 4 || type.columns == 4)
                format = Format::eR32G32B32A32Uint;
        }

        uint32_t byteSize = type.width / 8;
        uint32_t componentCount = 1;

        if (type.vecsize > 0)
            byteSize *= type.vecsize;

        if (type.columns > 0)
            componentCount = type.columns;

        return ShaderType{ format, componentCount, byteSize };
    }

    ShaderDescription& Reflect(ShaderDescription& description)
    {
        switch (description.stage)
        {
            case ShaderStage::eVertex:
                LOG_TRACE("[ VERTEX SHADER ]");
                break;
            case ShaderStage::eFragment:
                LOG_TRACE("[ FRAGMENT SHADER ]");
                break;
            default:
                break;
        }

        spirv_cross::Compiler compiler(description.byteCode);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        /// Stage inputs
        auto& inputAttributes = resources.stage_inputs;
        // sort in location order
        std::sort(inputAttributes.begin(), inputAttributes.end(), [&compiler](const auto& v1, const auto& v2)
            {
                return compiler.get_decoration(v1.id, spv::Decoration::DecorationLocation) <
                    compiler.get_decoration(v2.id, spv::Decoration::DecorationLocation);
            });

        for (const auto& inputAttribute : inputAttributes)
        {
            description.inputAttributes.push_back(GetTypeByReflection(compiler, inputAttribute));
        }

        LOG_TRACE("UNIFORM BUFFERS:");
        for (auto& uniformBuffer : resources.uniform_buffers)
        {
            auto& uniform = description.uniforms.emplace_back();
            uniform.descriptorCount = 1;
            uniform.descriptorType = DescriptorType::eUniformBuffer;
            uniform.binding = compiler.get_decoration(uniformBuffer.id, spv::Decoration::DecorationBinding);

            for (auto count : compiler.get_type(uniformBuffer.type_id).array)
                uniform.descriptorCount *= count;
            LOG_TRACE("Name: {}", uniformBuffer.name);
            LOG_TRACE("Binding: {}", uniform.binding);
            LOG_TRACE("Descriptor count: {}", uniform.descriptorCount);
            LOG_TRACE("Width: {}", compiler.get_type(uniformBuffer.base_type_id).width);
            LOG_TRACE("Vecsize: {}", compiler.get_type(uniformBuffer.base_type_id).vecsize);
            LOG_TRACE("Columns: {}", compiler.get_type(uniformBuffer.base_type_id).columns);
        }

        LOG_TRACE("SEPARATE SAMPLERS:");

        for (auto& sampler : resources.separate_samplers)
        {
            auto& uniform = description.uniforms.emplace_back();
            uniform.descriptorType = DescriptorType::eSampler;
            uniform.binding = compiler.get_decoration(sampler.id, spv::Decoration::DecorationBinding);

            for (auto count : compiler.get_type(sampler.type_id).array)
                uniform.descriptorCount *= count;

            LOG_TRACE("Name: {}", sampler.name);
            LOG_TRACE("Set: {}", compiler.get_decoration(sampler.id, spv::Decoration::DecorationDescriptorSet));
            LOG_TRACE("Binding: {}", compiler.get_decoration(sampler.id, spv::Decoration::DecorationBinding));
            LOG_TRACE("Width: {}", compiler.get_type(sampler.base_type_id).width);
            LOG_TRACE("Vecsize: {}", compiler.get_type(sampler.base_type_id).vecsize);
            LOG_TRACE("Columns: {}", compiler.get_type(sampler.base_type_id).columns);
        }

        LOG_TRACE("SEPARATE IMAGES:");

        for (auto& image : resources.separate_images)
        {
            auto& uniform = description.uniforms.emplace_back();
            uniform.descriptorType = DescriptorType::eSampledImage;
            uniform.binding = compiler.get_decoration(image.id, spv::Decoration::DecorationBinding);

            // Here I try to count descriptors if it's array
            for (auto count : compiler.get_type(image.type_id).array)
                uniform.descriptorCount *= count;

            LOG_TRACE("Name: {}", image.name);
            LOG_TRACE("Set: {}", compiler.get_decoration(image.id, spv::Decoration::DecorationDescriptorSet));
            LOG_TRACE("Binding: {}", compiler.get_decoration(image.id, spv::Decoration::DecorationBinding));
            LOG_TRACE("Width: {}", compiler.get_type(image.base_type_id).width);
            LOG_TRACE("Vecsize: {}", compiler.get_type(image.base_type_id).vecsize);
            LOG_TRACE("Columns: {}", compiler.get_type(image.base_type_id).columns);
        }

        LOG_TRACE("STORAGE IMAGES:");

        for (auto& image : resources.storage_images)
        {
            auto& uniform = description.uniforms.emplace_back();
            uniform.descriptorType = DescriptorType::eStorageImage;
            uniform.binding = compiler.get_decoration(image.id, spv::Decoration::DecorationBinding);

            // Here I try to count descriptors if it's array
            for (auto count : compiler.get_type(image.type_id).array)
                uniform.descriptorCount *= count;

            LOG_TRACE("Name: {}", image.name);
            LOG_TRACE("Set: {}", compiler.get_decoration(image.id, spv::Decoration::DecorationDescriptorSet));
            LOG_TRACE("Binding: {}", compiler.get_decoration(image.id, spv::Decoration::DecorationBinding));
            LOG_TRACE("Width: {}", compiler.get_type(image.base_type_id).width);
            LOG_TRACE("Vecsize: {}", compiler.get_type(image.base_type_id).vecsize);
            LOG_TRACE("Columns: {}", compiler.get_type(image.base_type_id).columns);
        }

        return description;
    }
} // namespace Fluent
