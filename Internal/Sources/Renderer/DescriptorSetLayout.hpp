#pragma once

#include <vector>
#include "Renderer/Shader.hpp"

namespace Fluent
{
    struct DescriptorSetLayoutDescription
    {
        std::vector<Ref<Shader>> shaders;
    };
} // namespace Fluent
