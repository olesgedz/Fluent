#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Fluent/Fluent.hpp"

using namespace Fluent;

struct Vertex
{
    Vector3 position;
    Vector3 normal;
    Vector2 texCoords;
    Vector3 tangent;
    Vector3 bitangent;
};

struct Mesh
{
    std::vector<Vertex>         vertices;
    std::vector<uint32_t>       indices;
    Ref<Buffer>                 vertexBuffer;
    Ref<Buffer>                 indexBuffer;
    Matrix4                     transform;

    void InitMesh()
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

    // constructor
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
        : vertices(std::move(vertices))
        , indices(std::move(indices))
    {
        InitMesh();
    }
};

class Model
{
public:
    struct LoadedTexture
    {
        std::string name;
        Ref<Image> texture;
    };
    // model data
    std::vector<LoadedTexture>  texturesLoaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Mesh>           meshes;
    std::string                 directory;

    // constructor, expects a filepath to a 3D model.
    Model(const std::string& filename)
        : directory(filename.substr(0, filename.find_last_of('/')))
    {
        LoadModel(filename);
    }
private:
    void LoadModel(const std::string& filename)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(FileSystem::GetModelsDirectory() + "/" + filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_WARN("ASSIMP ERROR: {}", importer.GetErrorString());
            return;
        }

        ProcessNode(scene->mRootNode, scene);
    }

    void ProcessNode(aiNode *node, const aiScene *scene)
    {
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(ProcessMesh(mesh, scene));
            auto& t = node->mTransformation;
            meshes.back().transform = Matrix4(t.a1, t.b1, t.c1, t.d1,
                                              t.a2, t.b2, t.c2, t.d2,
                                              t.a3, t.b3, t.c3, t.d3,
                                              t.a4, t.b4, t.c4, t.d4);
        }

        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene);
        }

    }

    Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene)
    {
        // data to fill
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<LoadedTexture> textures;

        // walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            Vector3 vector;
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.normal = vector;
            }
            // texture coordinates
            if(mesh->mTextureCoords[0])
            {
                glm::vec2 vec;

                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.texCoords = vec;

                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.bitangent = vector;
            }
            else
                vertex.texCoords = Vector2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        for (uint32_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            for (uint32_t j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // 1. diffuse maps
        std::vector<LoadedTexture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        std::vector<LoadedTexture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<LoadedTexture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<LoadedTexture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices);
    }

    std::vector<LoadedTexture> LoadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
        std::vector<LoadedTexture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            for (unsigned int j = 0; j < texturesLoaded.size(); j++)
            {
                if (texturesLoaded[j].name == typeName)
                {
                    textures.push_back(texturesLoaded[j]);
                    skip = true;
                    break;
                }
            }

            if (!skip)
            {
                std::string texName = str.C_Str();
                texName = texName.substr(0, texName.find_last_of('.')) + ".ktx";
                ImageDescription imageDesc{};
                imageDesc.initialUsage = ImageUsage::Bits::eSampled;
                imageDesc.filename = directory + "/" + texName;

                LoadedTexture texture{};
                texture.name = typeName;
                texture.texture = Image::Create(imageDesc);

                textures.push_back(texture);
                texturesLoaded.push_back(texture);
            }
        }
        return textures;
    }
};

struct CameraUBO
{
    Matrix4 projection;
    Matrix4 view;
};

class ParallaxMappingLayer : public Layer
{
private:
    Ref<Image>                  mRenderImage;
    Ref<Image>                  mDepthImage;
    Ref<RenderPass>             mRenderPass;
    Ref<Framebuffer>            mFramebuffer;
    Ref<Pipeline>               mPipeline;
    Ref<Buffer>                 mUniformBuffer;
    Ref<DescriptorSetLayout>    mDescriptorSetLayout;
    Ref<DescriptorSet>          mDescriptorSet;
    Ref<Sampler>                mSampler;

    Ref<Model>                  mModel;

    Timer                       mTimer;

    CameraUBO                   mCameraUBO;
public:
    ParallaxMappingLayer() : Layer("VertexBuffer") {}

    void CreateUniformBuffer()
    {
        auto& window = Application::Get().GetWindow();

        mCameraUBO.projection   = CreatePerspectiveMatrix(Radians(45.0f), window->GetAspect(), 0, 100.f);
        mCameraUBO.view         = CreateLookAtMatrix(Vector3(0.0f, 0.0, 2.0f), Vector3(0.0, 0.0, -1.0), Vector3(0.0, 1.0, 0.0));

        BufferDescription bufferDesc{};
        bufferDesc.bufferUsage = BufferUsage::eUniformBuffer;
        bufferDesc.memoryUsage = MemoryUsage::eCpuToGpu;
        bufferDesc.size = sizeof(CameraUBO);

        mUniformBuffer = Buffer::Create(bufferDesc);
        mUniformBuffer->WriteData(&mCameraUBO, bufferDesc.size, 0);
    }

    void CreateSampler()
    {
        SamplerDescription samplerDesc{};
        samplerDesc.mipmapMode = SamplerMipmapMode::eLinear;
        samplerDesc.minLod = 0;
        samplerDesc.maxLod = 1000;
        mSampler = Sampler::Create(samplerDesc);
    }

    void OnAttach() override
    {
        FileSystem::SetShadersDirectory("../../../Internal/Examples/Shaders/");
        FileSystem::SetTexturesDirectory("../../../Internal/Examples/Textures/");
        FileSystem::SetModelsDirectory("../../../Internal/Examples/Models");

        auto& window = Application::Get().GetWindow();

        ClearValue clearValue{};
        clearValue.color = Vector4(0.0, 0.0, 0.0, 1.0);

        RenderPassDescription renderPassDesc{};
        renderPassDesc.width = window->GetWidth();
        renderPassDesc.height = window->GetHeight();
        renderPassDesc.clearValues = { clearValue };
        renderPassDesc.colorFormats = { Format::eR8G8B8A8Unorm };
        renderPassDesc.initialUsages = { ImageUsage::eUndefined, ImageUsage::eUndefined };
        renderPassDesc.finalUsages = { ImageUsage::eStorage, ImageUsage::eDepthStencilAttachment };
        renderPassDesc.attachmentLoadOps = { AttachmentLoadOp::eClear };
        renderPassDesc.depthStencilFormat = Format::eD32Sfloat;
        renderPassDesc.depthLoadOp = AttachmentLoadOp::eClear;
        renderPassDesc.stencilLoadOp = AttachmentLoadOp::eClear;
        renderPassDesc.sampleCount = SampleCount::e1;

        mRenderPass = RenderPass::Create(renderPassDesc);
        
        ShaderDescription vertexShaderDesc{};
        vertexShaderDesc.stage = ShaderStage::eVertex;
        vertexShaderDesc.filename = "08_ModelLoading/main.vert.glsl";

        ShaderDescription fragmentShaderDesc{};
        fragmentShaderDesc.stage = ShaderStage::eFragment;
        fragmentShaderDesc.filename = "08_ModelLoading/main.frag.glsl";

        auto vertexShader = Shader::Create(vertexShaderDesc);
        auto fragmentShader = Shader::Create(fragmentShaderDesc);
        
        DescriptorSetLayoutDescription descriptorSetLayoutDesc{};
        descriptorSetLayoutDesc.shaders = { vertexShader, fragmentShader };

        mDescriptorSetLayout = DescriptorSetLayout::Create(descriptorSetLayoutDesc);

        RasterizerStateDescription rasterizerState{};
        rasterizerState.cullMode = CullMode::eBack;
        rasterizerState.frontFace = FrontFace::eCounterClockwise;

        DepthStateDescription depthStateDescription{};
        depthStateDescription.depthTest = true;
        depthStateDescription.depthWrite = true;
        depthStateDescription.compareOp = CompareOp::eLess;

        PipelineDescription pipelineDesc{};
        pipelineDesc.type = PipelineType::eGraphics;
        pipelineDesc.bindingDescriptions =
        {
            VertexBindingDescription
            {
                0,
                sizeof(Vertex),
                VertexInputRate::eVertex
            }
        };

        pipelineDesc.attributeDescriptions = 
        {
            VertexAttributeDescription
            {
                0,
                0,
                Format::eR32G32B32Sfloat,
                offsetof(Vertex, position)
            },
            {
                1,
                0,
                Format::eR32G32B32Sfloat,
                offsetof(Vertex, normal)
            },
            {
                2,
                0,
                Format::eR32G32Sfloat,
                offsetof(Vertex, texCoords)
            },
            {
                3,
                0,
                Format::eR32G32B32Sfloat,
                offsetof(Vertex, tangent)
            },
            {
                4,
                0,
                Format::eR32G32B32Sfloat,
                offsetof(Vertex, bitangent)
            }
        };

        pipelineDesc.descriptorSetLayout = mDescriptorSetLayout;
        pipelineDesc.rasterizerDescription = rasterizerState;
        pipelineDesc.renderPass = mRenderPass;

        mPipeline = Pipeline::Create(pipelineDesc);

        CreateUniformBuffer();
        CreateSampler();

        mModel = CreateScope<Model>("backpack/backpack.obj");

        DescriptorSetDescription descriptorSetDesc{};
        descriptorSetDesc.descriptorSetLayout = mDescriptorSetLayout;
        mDescriptorSet = DescriptorSet::Create(descriptorSetDesc);

        std::vector<ImageUpdateDesc> imageUpdates(mModel->texturesLoaded.size());
        for (uint32_t i = 0; i < imageUpdates.size(); ++i)
        {
            imageUpdates[i].image = mModel->texturesLoaded[i].texture;
            imageUpdates[i].usage = ImageUsage::eSampled;
        }

        BufferUpdateDesc bufferUpdateDesc{};
        bufferUpdateDesc.buffer = mUniformBuffer;
        bufferUpdateDesc.offset = 0;
        bufferUpdateDesc.range = sizeof(CameraUBO);

        std::vector<DescriptorSetUpdateDesc> updateDescriptions(3);
        updateDescriptions[0].binding = 0;
        updateDescriptions[0].bufferUpdates = { bufferUpdateDesc };
        updateDescriptions[0].descriptorType = DescriptorType::eUniformBuffer;

        ImageUpdateDesc imageUpdate {};
        imageUpdate.sampler = mSampler;
        updateDescriptions[1].binding = 1;
        updateDescriptions[1].imageUpdates = { imageUpdate };
        updateDescriptions[1].descriptorType = DescriptorType::eSampler;

        updateDescriptions[2].binding = 2;
        updateDescriptions[2].imageUpdates = imageUpdates;
        updateDescriptions[2].descriptorType = DescriptorType::eSampledImage;

        mDescriptorSet->UpdateDescriptorSet(updateDescriptions);
    }

    void OnDetach() override
    {
        mUniformBuffer = nullptr;
        mPipeline = nullptr;
        mFramebuffer = nullptr;
        mRenderImage = nullptr;
    }

    void OnLoad() override
    {
        auto& window = Application::Get().GetWindow();

        ImageDescription imageDesc{};
        imageDesc.arraySize = 1;
        imageDesc.mipLevels = 1;
        imageDesc.depth = 1;
        imageDesc.format = Format::eR8G8B8A8Unorm;
        imageDesc.width = window->GetWidth();
        imageDesc.height = window->GetHeight();
        imageDesc.initialUsage = ImageUsage::Bits::eStorage;

        mRenderImage = Image::Create(imageDesc);

        imageDesc.format = Format::eD32Sfloat;
        imageDesc.initialUsage = ImageUsage::Bits::eDepthStencilAttachment;

        mDepthImage = Image::Create(imageDesc);

        FramebufferDescription framebufferDesc {};
        framebufferDesc.width = window->GetWidth();
        framebufferDesc.height = window->GetHeight();
        framebufferDesc.renderPass = mRenderPass;
        framebufferDesc.targets = {{ mRenderImage }};
        framebufferDesc.depthStencil = mDepthImage;

        mFramebuffer = Framebuffer::Create(framebufferDesc);

        mRenderPass->SetRenderArea(window->GetWidth(), window->GetHeight());

        mCameraUBO.projection = CreatePerspectiveMatrix(Radians(45.0f), window->GetAspect(), 0, 100.f);
    }

    void OnUnload() override
    {
        mFramebuffer = nullptr;
        mRenderImage = nullptr;
        mDepthImage = nullptr;
    }

    void OnUpdate(float deltaTime) override
    {
        mUniformBuffer->WriteData(&mCameraUBO, sizeof(CameraUBO), 0);

        auto& context = Application::Get().GetGraphicContext();
        auto& window = Application::Get().GetWindow();

        auto cmd = context->GetCurrentCommandBuffer();
        cmd->BeginRenderPass(mRenderPass, mFramebuffer);
        cmd->SetViewport(window->GetWidth(), window->GetHeight(), 0.0f, 1.0f, 0, 0);
        cmd->SetScissor(window->GetWidth(), window->GetHeight(), 0, 0);
        cmd->BindDescriptorSet(mPipeline, mDescriptorSet);
        cmd->BindPipeline(mPipeline);
        for (uint32_t i = 0; i < mModel->meshes.size(); i++)
        {
            auto transform = mModel->meshes[i].transform;
            transform = glm::scale(transform, Vector3(0.4));
            cmd->PushConstants(mPipeline, 0, sizeof(Matrix4), &transform);
            cmd->BindVertexBuffer(mModel->meshes[i].vertexBuffer, 0);
            cmd->BindIndexBuffer(mModel->meshes[i].indexBuffer, 0, IndexType::eUint32);
            cmd->DrawIndexed(mModel->meshes[i].indices.size(), 1, 0, 0, 0);
        }
        cmd->EndRenderPass();
        uint32_t activeImage = context->GetActiveImageIndex();
        auto swapchainImageUsage = context->GetSwapchainImageUsage(activeImage);
        auto swapchainImage = context->AcquireImage(activeImage, ImageUsage::eTransferDst);
        cmd->BlitImage(mRenderImage, ImageUsage::eStorage, swapchainImage, swapchainImageUsage, Filter::eLinear);
    }
};

int main(int argc, char** argv)
{
    Log::SetLogLevel(Log::LogLevel::eTrace);
        
    WindowDescription windowDescription {};
    windowDescription.width = 800;
    windowDescription.height = 600;
    
    ApplicationDescription appDesc {};
    appDesc.argv = argv;
    appDesc.windowDescription = windowDescription;
    appDesc.askGraphicValidation = true;
    
    Application app(appDesc);
    ParallaxMappingLayer layer;
    app.PushLayer(layer);
    app.Run();

    return 0;
}
