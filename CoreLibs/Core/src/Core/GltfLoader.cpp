#include "GltfLoader.h"
#include <tiny_gltf.h>
#include "File.h"
#include <Log.h>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>
#include "../Engine/Geometry/Mesh.h"
#include "../Engine/Geometry/Material.h"

namespace FooGame
{
    static std::vector<GltfImageSource> ProcessGLTFImages(std::vector<tinygltf::Image>& images);

    static std::vector<Material> ProcessGLTFMaterial(const tinygltf::Model& gltfModel,
                                                     std::string defaultBaseColorTextureName);

    bool ReadFile(tinygltf::Model& input, const std::string& path, bool isGlb);
    GltfLoader::GltfLoader(const std::string& path, const std::string& name, bool isGlb)
        : m_Path(path), m_Name(name), m_IsGlb(isGlb)
    {
        if (m_Name.empty() || m_Path.empty())
        {
            FOO_ENGINE_WARN("Name or path is empty, name: {0}, path {1}", m_Name, m_Path);
            m_EligibleToLoad = false;
        }
        else
        {
            m_EligibleToLoad = true;
        }
    }
    GltfModel* GltfLoader::Load() const
    {
        if (!m_EligibleToLoad)
        {
            return {};
        }
        GltfModel* model = new GltfModel;
        model->Name      = File::ExtractFileName(m_Path);

        tinygltf::Model gltfInput;
        std::vector<Mesh> meshes;
        if (!ReadFile(gltfInput, m_Path, m_IsGlb))
        {
            FOO_ENGINE_ERROR("Model with path: {0} could not read", m_Path);
            return {};
        }
        model->ImageSources = std::move(ProcessGLTFImages(gltfInput.images));

        model->Materials = std::move(ProcessGLTFMaterial(gltfInput, std::string()));

        if (gltfInput.scenes.size() > 1)
        {
            FOO_ENGINE_WARN("GLTF input has more than 1 scene which is not supported");
        }

        for (size_t nodeIndex = 0; nodeIndex < gltfInput.nodes.size(); nodeIndex++)
        {
            const auto& node     = gltfInput.nodes[nodeIndex];
            const auto meshIndex = node.mesh;
            if (meshIndex <= -1)
            {
                continue;
            }
            const auto mesh = gltfInput.meshes[meshIndex];
            Mesh tempMesh;
            tempMesh.Name = mesh.name;
            for (const auto& primitive : mesh.primitives)
            {
                auto material             = model->Materials[primitive.material];
                tempMesh.M3Name           = material.Name;
                uint32_t firstIndex       = static_cast<uint32_t>(tempMesh.m_Indices.size());
                uint32_t vertexStart      = static_cast<uint32_t>(tempMesh.m_Vertices.size());
                uint32_t indexCount       = 0;
                const auto indicesIndex   = primitive.indices;
                const auto materialIndex  = primitive.material;
                const auto positionsIndex = primitive.attributes.at("POSITION");
                const auto uvsIndex       = primitive.attributes.at("TEXCOORD_0");
                const auto normalsIndex   = primitive.attributes.at("NORMAL");

                const auto indicesAccessor   = gltfInput.accessors[indicesIndex];
                const auto positionsAccessor = gltfInput.accessors[positionsIndex];
                const auto uvsAccessor       = gltfInput.accessors[uvsIndex];
                const auto normalsAccessor   = gltfInput.accessors[normalsIndex];

                const auto& indicesBufferView = gltfInput.bufferViews[indicesAccessor.bufferView];
                const auto& positionBV        = gltfInput.bufferViews[positionsAccessor.bufferView];
                const auto& uvsBV             = gltfInput.bufferViews[uvsAccessor.bufferView];
                const auto& normalsBV         = gltfInput.bufferViews[normalsAccessor.bufferView];
                const float* positionsBuffer  = reinterpret_cast<const float*>(
                    &(gltfInput.buffers[positionBV.buffer]
                          .data[positionsAccessor.byteOffset + positionBV.byteOffset]));

                const float* normalsBuffer = reinterpret_cast<const float*>(
                    &(gltfInput.buffers[normalsBV.buffer]
                          .data[normalsAccessor.byteOffset + normalsBV.byteOffset]));

                const float* uvsBuffer = reinterpret_cast<const float*>(
                    &(gltfInput.buffers[uvsBV.buffer]
                          .data[uvsAccessor.byteOffset + uvsBV.byteOffset]));

                const float* indicesBuffer = reinterpret_cast<const float*>(
                    &(gltfInput.buffers[indicesBufferView.buffer]
                          .data[indicesAccessor.byteOffset + indicesBufferView.byteOffset]));

                tempMesh.m_Vertices.reserve(positionsAccessor.count);
                for (size_t w = 0; w < positionsAccessor.count; w++)
                {
                    FooGame::Vertex v{};

                    v.Position = glm::vec4(glm::make_vec3(&positionsBuffer[w * 3]), 1.0f);
                    v.Normal   = glm::normalize(glm::vec3(
                        normalsBuffer ? glm::make_vec3(&normalsBuffer[w * 3]) : glm::vec3(0.0f)));
                    v.TexCoord =
                        uvsBuffer ? glm::vec2(glm::make_vec2(&uvsBuffer[w * 2])) : glm::vec2(0.0f);
                    tempMesh.m_Vertices.push_back(v);
                }
                {
                    const auto indexCount = static_cast<uint32_t>(indicesAccessor.count);
                    switch (indicesAccessor.componentType)
                    {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                        {
                            const auto& indicesBuff = gltfInput.buffers[indicesBufferView.buffer];
                            const uint32_t* buf     = reinterpret_cast<const uint32_t*>(
                                &indicesBuff.data[indicesAccessor.byteOffset +
                                                  indicesBufferView.byteOffset]);
                            for (size_t index = 0; index < indicesAccessor.count; index++)
                            {
                                tempMesh.m_Indices.push_back(buf[index] + vertexStart);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                        {
                            const auto& indicesBuff = gltfInput.buffers[indicesBufferView.buffer];
                            const uint16_t* buf     = reinterpret_cast<const uint16_t*>(
                                &indicesBuff.data[indicesAccessor.byteOffset +
                                                  indicesBufferView.byteOffset]);
                            // indexCount +=
                            // static_cast<uint32_t>(accessor.count);
                            for (size_t index = 0; index < indicesAccessor.count; index++)
                            {
                                tempMesh.m_Indices.push_back(buf[index] + vertexStart);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                        {
                            const auto& indicesBuff = gltfInput.buffers[indicesBufferView.buffer];
                            const uint8_t* buf      = reinterpret_cast<const uint8_t*>(
                                &indicesBuff.data[indicesAccessor.byteOffset +
                                                  indicesBufferView.byteOffset]);
                            for (size_t index = 0; index < indicesAccessor.count; index++)
                            {
                                tempMesh.m_Indices.push_back(buf[index] + vertexStart);
                            }
                            break;
                        }
                        default:
                            FOO_CORE_WARN(
                                "Index component type {0} not "
                                "sopperted",
                                indicesAccessor.componentType);
                            break;
                    }
                }
            }
            meshes.emplace_back(std::move(tempMesh));
        }
        for (auto& m : meshes)
        {
            model->Meshes.emplace_back(std::move(m));
        }
        return model;
    }

    std::vector<GltfImageSource> ProcessGLTFImages(std::vector<tinygltf::Image>& images)
    {
        std::vector<GltfImageSource> imageSources;
        for (auto& gltfImage : images)
        {
            unsigned char* imageBuffer = nullptr;
            size_t imageSize           = 0;
            bool deleteBuffer          = false;

            if (gltfImage.component == 3)
            {
                imageSize           = gltfImage.width * gltfImage.height * 4;
                imageBuffer         = new unsigned char[imageSize];
                unsigned char* rgba = imageBuffer;
                unsigned char* rgb  = &gltfImage.image[0];
                for (size_t j = 0; j < gltfImage.width * gltfImage.height; j++)
                {
                    memcpy(rgba, rgb, sizeof(unsigned char) * 3);
                    rgba += 4;
                    rgb  += 3;
                }
                deleteBuffer = true;
            }
            else
            {
                imageBuffer = &gltfImage.image[0];
                imageSize   = gltfImage.image.size();
            }
            GltfImageSource source{};
            auto imageName = gltfImage.name.empty() ? gltfImage.uri : gltfImage.name;
            auto fileName  = File::ExtractFileName(imageName);

            source.ImageBuffer = new unsigned char[imageSize];
            memcpy(source.ImageBuffer, (void*)imageBuffer, imageSize);
            source.ImageSize      = imageSize;
            source.Width          = gltfImage.width;
            source.Height         = gltfImage.height;
            source.ComponentCount = gltfImage.component;
            if (fileName.empty())
            {
                source.Name = "UnNamed Image";
            }
            else
            {
                source.Name = fileName;
            }
            imageSources.push_back(source);
        }
        return imageSources;
    }
    std::vector<Material> ProcessGLTFMaterial(const tinygltf::Model& gltfModel,
                                              std::string defaultBaseColorTextureName)
    {
        std::vector<Material> materials;

        const auto& gMats     = gltfModel.materials;
        const auto& gImages   = gltfModel.images;
        const auto& gTextures = gltfModel.textures;
        for (auto& m : gMats)
        {
            Material mt;
            mt.fromGlb = true;
            mt.Name    = m.name;
            if (m.pbrMetallicRoughness.baseColorTexture.index != -1)
            {
                auto& texture = gTextures[m.pbrMetallicRoughness.baseColorTexture.index];

                auto gBaseColorTexture = gImages[texture.source];
                auto baseColorTextureName =
                    gBaseColorTexture.name.empty() ? gBaseColorTexture.uri : gBaseColorTexture.name;
                mt.PbrMat.BaseColorTextureName = File::ExtractFileName(baseColorTextureName);

                mt.PbrMat.BaseColorFactor[0] = m.pbrMetallicRoughness.baseColorFactor[0];
                mt.PbrMat.BaseColorFactor[1] = m.pbrMetallicRoughness.baseColorFactor[1];
                mt.PbrMat.BaseColorFactor[2] = m.pbrMetallicRoughness.baseColorFactor[2];
                mt.PbrMat.BaseColorFactor[3] = m.pbrMetallicRoughness.baseColorFactor[3];

                mt.PbrMat.MetallicFactor       = m.pbrMetallicRoughness.metallicFactor;
                mt.PbrMat.RoughnessFactor      = m.pbrMetallicRoughness.roughnessFactor;
                mt.PbrMat.BaseColorTexturePath = gBaseColorTexture.uri;
            }
            if (m.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
            {
                auto& texture = gTextures[m.pbrMetallicRoughness.metallicRoughnessTexture.index];

                auto& metallicRoughness = gImages[texture.source];
                mt.PbrMat.MetallicRoughnessTextureName =
                    metallicRoughness.name.empty() ? File::ExtractFileName(metallicRoughness.uri)
                                                   : metallicRoughness.name;
                mt.PbrMat.MetallicRoughnessTexturePath = metallicRoughness.uri;
            }
            if (m.normalTexture.index != -1)
            {
                auto& texture       = gTextures[m.normalTexture.index];
                auto& normalTexture = gImages[texture.source];
                auto normalTexturename =
                    normalTexture.name.empty() ? normalTexture.uri : normalTexture.name;
                mt.NormalTexture.Name = File::ExtractFileName(normalTexturename);
            }
            materials.push_back(mt);
        }

        return materials;
    }
    bool ReadFile(tinygltf::Model& input, const std::string& path, bool isGlb)
    {
        std::string error, warning;
        tinygltf::TinyGLTF gltfContext;

        bool ret;
        if (isGlb)
        {
            ret = gltfContext.LoadBinaryFromFile(&input, &error, &warning, path);
        }
        else
        {
            ret = gltfContext.LoadASCIIFromFile(&input, &error, &warning, path);
        }

        if (!warning.empty())
        {
            FOO_CORE_WARN("Warn while loading GLTF model {0}", warning.c_str());
        }

        if (!error.empty())
        {
            FOO_CORE_ERROR("Err: {0}", error.c_str());
        }

        if (!ret)
        {
            FOO_CORE_ERROR("Failed to parse glTF : {0}", path);
        }
        return ret;
    }
}  // namespace FooGame
