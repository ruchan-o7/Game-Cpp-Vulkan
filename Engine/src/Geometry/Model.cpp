#include "Model.h"
#include "../Engine/Api.h"
#include "../Engine/Device.h"
#include "../Engine/VulkanCheckResult.h"
namespace Engine
{
    Model::Model(List<Mesh>&& meshes) : m_Meshes(std::move(meshes))
    {
    }
    Mesh::Mesh(List<Vertex>&& vertices, List<u32>&& indices)
        : m_Vertices(std::move(vertices)), m_Indices(std::move(indices))
    {
        auto device = Api::GetDevice();
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding            = 0;
        uboLayoutBinding.descriptorCount    = 1;
        uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding         = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers  = nullptr;
        samplerLayoutBinding.stageFlags          = VK_SHADER_STAGE_FRAGMENT_BIT;
        VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding,
                                                    samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = ARRAY_COUNT(bindings);
        layoutInfo.pBindings    = bindings;

        VK_CALL(vkCreateDescriptorSetLayout(device->GetDevice(), &layoutInfo,
                                            nullptr, &m_Layout));
    }
    // Shared<Model> Model::LoadModel(const String& path)
    // {
    //     tinyobj::attrib_t attrib;
    //     List<tinyobj::shape_t> shapes;
    //     List<tinyobj::material_t> materials;
    //     String warn, err;
    //     List<Vertex> vertices;
    //     List<u32> indices;
    //
    //     if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
    //                           path.c_str()))
    //     {
    //         assert(0 && "Model could not loaded!!!");
    //     }
    //     for (const auto& shape : shapes)
    //     {
    //         for (const auto& index : shape.mesh.indices)
    //         {
    //             Vertex vertex{};
    //             vertex.Position = {
    //                 attrib.vertices[3 * index.vertex_index + 0],
    //                 attrib.vertices[3 * index.vertex_index + 1],
    //                 attrib.vertices[3 * index.vertex_index + 2],
    //             };
    //             vertex.TexCoord = {
    //                 attrib.texcoords[2 * index.texcoord_index + 0],
    //                 1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
    //             };
    //             vertex.Color = {
    //                 attrib.colors[3 * index.vertex_index + 0],
    //                 attrib.colors[3 * index.vertex_index + 1],
    //                 attrib.colors[3 * index.vertex_index + 2],
    //             };  // im not sure
    //
    //             vertices.push_back(vertex);
    //             indices.push_back(indices.size());
    //         }
    //     }
    //     List<Mesh> meshes;
    //     meshes.push_back({std::move(vertices), std::move(indices)});
    //     return CreateShared<Model>(std::move(meshes));
    // }
    void Mesh::UpdateDescriptorBuffer(const VkBuffer& buffer, VkDevice device,
                                      u32 index)
    {
        // for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrites[2] = {};
        descriptorWrites[0].sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet     = m_DescriptorSets[index];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo     = &bufferInfo;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = m_Texture->ImageView;
        imageInfo.sampler     = m_Texture->Sampler;

        descriptorWrites[1].sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet     = m_DescriptorSets[index];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo      = &imageInfo;

        vkUpdateDescriptorSets(device, ARRAY_COUNT(descriptorWrites),
                               descriptorWrites, 0, nullptr);
    }

}  // namespace Engine
