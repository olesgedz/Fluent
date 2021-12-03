#pragma once

#include <cstdint>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Renderer/Image.hpp"
#include "Scene/Model.hpp"

namespace Fluent
{
    struct LoadModelDescription
    {
        std::string filename;
        bool loadNormals;
        bool loadTexCoords;
        bool loadTangents;
        bool loadBitangents;
    };

    class ModelLoader
    {
    private:
        static constexpr float mNormalComponentCount = 3;
        static constexpr float mTexCoordComponentCount = 2;
        static constexpr float mTangentsComponentCount = 3;
        static constexpr float mBitangentsComponentCount = 3;

        int mNormalOffset = -1;
        int mTexCoordOffset = -1;
        int mTangentsOffset = -1;
        int mBitangentsOffset = -1;

        struct LoadedTexture
        {
            std::string name;
            Ref<Image> texture;
        };

        uint32_t                    mStride = 0;
        std::vector<LoadedTexture>  mTexturesLoaded;
        std::string                 mDirectory;

        [[nodiscard]]
        Model Load(const std::string& filename);

        void ProcessNode(Model& model, aiNode *node, const aiScene *scene);

        Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene);

        std::vector<LoadedTexture> LoadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

        void CountStride(const LoadModelDescription& desc);

    public:
        [[nodiscard]]
        Model Load(const LoadModelDescription& desc);

        std::vector<VertexBindingDescription> GetVertexBindingDescription();
        std::vector<VertexAttributeDescription> GetVertexAttributeDescription();
    };
}