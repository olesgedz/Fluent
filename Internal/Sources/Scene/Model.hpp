#pragma once

#include <vector>
#include "Renderer/Buffer.hpp"
#include "Math/Math.hpp"

namespace Fluent
{
    struct TextureIndices
    {
        int diffuse = -1;
        int specular = -1;
        int normal = -1;
        int height = -1;
    };

    struct Material
    {
        TextureIndices textureIndices;
    };

    struct Mesh
    {
        std::vector<float>          vertices;
        std::vector<uint32_t>       indices;
        Ref<Buffer>                 vertexBuffer;
        Ref<Buffer>                 indexBuffer;
        Matrix4                     transform;
        Material                    material;

        void InitMesh();

        Mesh(std::vector<float> vertices, std::vector<uint32_t> indices, Material material);
    };

    struct Model
    {
        std::vector<Mesh> meshes;
        std::vector<Ref<Image>> textures;
    };
}