#pragma once

#include <vector>
#include "Renderer/Shader.hpp"

namespace Fluent
{
    struct DescriptorSetLayoutDescription
    {
        std::vector<Ref<Shader>> shaders;
    };

    class DescriptorSetLayout
    {
    protected:
        DescriptorSetLayout() = default;
    public:
        virtual ~DescriptorSetLayout() = default;

        virtual const std::vector<Ref<Shader>>& GetShaders() const = 0;
        virtual Handle GetNativeHandle() const = 0;
        
        static Ref<DescriptorSetLayout> Create(const DescriptorSetLayoutDescription& description);
    };
} // namespace Fluent
