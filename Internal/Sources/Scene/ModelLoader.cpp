#include "Scene/ModelLoader.hpp"
#include "Core/FileSystem.hpp"

namespace Fluent
{
    void ModelLoader::CountStride(const LoadModelDescription& desc)
    {
        mStride += 3;
        if (desc.loadNormals)
        {
            mNormalOffset = static_cast<int>(mStride);
            mStride += mNormalComponentCount;
        }

        if (desc.loadTexCoords)
        {
            mTexCoordOffset = static_cast<int>(mStride);
            mStride += mTexCoordComponentCount;
        }

        if (desc.loadTangents)
        {
            mTangentsOffset = static_cast<int>(mStride);
            mStride += mTangentsComponentCount;
        }

        if (desc.loadBitangents)
        {
            mBitangentsOffset = static_cast<int>(mStride);
            mStride += mBitangentsComponentCount;
        }
    }

    Model ModelLoader::Load(const LoadModelDescription& desc)
    {
        mDirectory = std::string(desc.filename.substr(0, desc.filename.find_last_of('/')));
        CountStride(desc);
        return Load(desc.filename);
    }

    Model ModelLoader::Load(const std::string& filename)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(FileSystem::GetModelsDirectory() + "/" + filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_WARN("ASSIMP ERROR: {}", importer.GetErrorString());
            return {};
        }

        Model model;
        ProcessNode(model, scene->mRootNode, scene);

        model.textures.resize(mTexturesLoaded.size());
        for (uint32_t i = 0; i < mTexturesLoaded.size(); ++i)
            model.textures[i] = mTexturesLoaded[i].texture;

        return model;
    }

    void ModelLoader::ProcessNode(Model& model, aiNode *node, const aiScene *scene)
    {
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            model.meshes.push_back(ProcessMesh(mesh, scene));
            auto& t = node->mTransformation;
            model.meshes.back().transform = Matrix4(t.a1, t.b1, t.c1, t.d1,
                                                    t.a2, t.b2, t.c2, t.d2,
                                                    t.a3, t.b3, t.c3, t.d3,
                                                    t.a4, t.b4, t.c4, t.d4);
        }

        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(model, node->mChildren[i], scene);
        }
    }

    Mesh ModelLoader::ProcessMesh(aiMesh *mesh, const aiScene *scene)
    {
        // data to fill
        LOG_INFO(mesh->mNumVertices * mStride);
        std::vector<float> vertices(mesh->mNumVertices * mStride);
        std::vector<uint32_t> indices;
        std::vector<LoadedTexture> textures;

        // walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            uint32_t idx = i * mStride;
            vertices[idx] = mesh->mVertices[i].x;
            vertices[idx + 1] = mesh->mVertices[i].y;
            vertices[idx + 2] = mesh->mVertices[i].z;
            // normals
            if (mesh->HasNormals() && mNormalOffset > -1)
            {
                idx = i * mStride + mNormalOffset;
                vertices[idx] = mesh->mNormals[i].x;
                vertices[idx + 1] = mesh->mNormals[i].y;
                vertices[idx + 2] = mesh->mNormals[i].z;
            }
            // texture coordinates
            if (mesh->mTextureCoords[0] && mTexCoordOffset > -1)
            {
                idx = i * mStride + mTexCoordOffset;
                vertices[idx] = mesh->mTextureCoords[0][i].x;
                vertices[idx + 1] = mesh->mTextureCoords[0][i].y;
            }
            else
            if (mTexCoordOffset > -1)
            {
                idx = i * mStride + mTexCoordOffset;
                vertices[idx] = 0;
                vertices[idx + 1] = 0;
            }

            if (mesh->HasTangentsAndBitangents())
            {
                if (mTangentsOffset > -1)
                {
                    idx = i * mStride + mTangentsOffset;
                    // tangent
                    vertices[idx] = mesh->mTangents[i].x;
                    vertices[idx + 1] = mesh->mTangents[i].y;
                    vertices[idx + 2] = mesh->mTangents[i].z;
                }

                if (mBitangentsOffset > -1)
                {
                    // bitangent
                    idx = i * mStride + mBitangentsOffset;
                    vertices[idx] = mesh->mBitangents[i].x;
                    vertices[idx + 1] = mesh->mBitangents[i].y;
                    vertices[idx + 2] = mesh->mBitangents[i].z;
                }
            }
        }

        for (uint32_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            for (uint32_t j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        Material meshMaterial{};

        int prevTexSize = textures.size();
        std::vector<LoadedTexture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        meshMaterial.textureIndices.diffuse = prevTexSize != textures.size() ? textures.size() - 1 : -1;
        prevTexSize = textures.size();

        std::vector<LoadedTexture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        meshMaterial.textureIndices.specular = prevTexSize != textures.size() ? textures.size() - 1 : -1;
        prevTexSize = textures.size();

        std::vector<LoadedTexture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        meshMaterial.textureIndices.normal = prevTexSize != textures.size() ? textures.size() - 1 : -1;
        prevTexSize = textures.size();

        std::vector<LoadedTexture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        meshMaterial.textureIndices.height = prevTexSize != textures.size() ? textures.size() - 1 : -1;
        prevTexSize = textures.size();

        return Mesh(vertices, indices, meshMaterial);
    }

    std::vector<ModelLoader::LoadedTexture> ModelLoader::LoadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
        std::vector<LoadedTexture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            for (unsigned int j = 0; j < mTexturesLoaded.size(); j++)
            {
                if (mTexturesLoaded[j].name == typeName)
                {
                    textures.push_back(mTexturesLoaded[j]);
                    skip = true;
                    break;
                }
            }

            if (!skip)
            {
                std::string texName = str.C_Str();
                texName = texName + ".ktx";
                LOG_INFO(texName);
                ImageDescription imageDesc{};
                imageDesc.initialUsage = ImageUsage::Bits::eSampled;
                imageDesc.filename = mDirectory + "/" + texName;

                LoadedTexture texture{};
                texture.name = typeName;
                texture.texture = Image::Create(imageDesc);

                textures.push_back(texture);
                mTexturesLoaded.push_back(texture);
            }
        }
        return textures;
    }

    /// Pipeline create helpers
    std::vector<VertexBindingDescription> ModelLoader::GetVertexBindingDescription()
    {
        VertexBindingDescription result;
        result.binding = 0;
        result.stride = mStride * sizeof(float);
        result.inputRate = VertexInputRate::eVertex;

        return { result };
    }

    std::vector<VertexAttributeDescription> ModelLoader::GetVertexAttributeDescription()
    {
        std::vector<VertexAttributeDescription> result;
        // Positions always exist
        {
            auto &desc = result.emplace_back();
            desc.location = 0;
            desc.binding = 0;
            desc.format = Format::eR32G32B32Sfloat;
            desc.offset = 0;
        }
        // Normals
        if (mNormalOffset > -1)
        {
            auto& desc = result.emplace_back();
            desc.location = static_cast<uint32_t>(result.size() - 1);
            desc.binding = 0;
            desc.offset = mNormalOffset * sizeof(float);
            desc.format = Format::eR32G32B32Sfloat;
        }
        // TexCoords
        if (mTexCoordOffset > -1)
        {
            auto& desc = result.emplace_back();
            desc.location = static_cast<uint32_t>(result.size() - 1);
            desc.binding = 0;
            desc.offset = mTexCoordOffset * sizeof(float);
            desc.format = Format::eR32G32Sfloat;
        }
        // Tangents
        if (mTangentsOffset > -1)
        {
            auto& desc = result.emplace_back();
            desc.location = static_cast<uint32_t>(result.size() - 1);
            desc.binding = 0;
            desc.offset = mTangentsOffset * sizeof(float);
            desc.format = Format::eR32G32B32Sfloat;
        }
        // Bitangents
        if (mBitangentsOffset > -1)
        {
            auto& desc = result.emplace_back();
            desc.location = static_cast<uint32_t>(result.size() - 1);
            desc.binding = 0;
            desc.offset = mBitangentsOffset * sizeof(float);
            desc.format = Format::eR32G32B32Sfloat;
        }

        return result;
    }
}