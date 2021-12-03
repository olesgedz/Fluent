#include "Scene/Model.hpp"

namespace Fluent
{
    void Mesh::InitMesh()
    {
        BufferDescription bufferDesc{};
        bufferDesc.bufferUsage = BufferUsage::eVertexBuffer;
        bufferDesc.memoryUsage = MemoryUsage::eGpu;
        bufferDesc.size = vertices.size() * sizeof(vertices[0]);
        bufferDesc.data = vertices.data();

        vertexBuffer = Buffer::Create(bufferDesc);

        bufferDesc = {};
        bufferDesc.bufferUsage = BufferUsage::eIndexBuffer;
        bufferDesc.memoryUsage = MemoryUsage::eGpu;
        bufferDesc.size = indices.size() * sizeof(indices[0]);
        bufferDesc.data = indices.data();

        indexBuffer = Buffer::Create(bufferDesc);
    }

    Mesh::Mesh(std::vector<float> vertices, std::vector<uint32_t> indices, Material material)
        : vertices(std::move(vertices))
        , indices(std::move(indices))
        , material(std::move(material))
    {
        InitMesh();
    }
}