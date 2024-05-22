#include "Renderer3D.h"
#include "Backend.h"
#include <cassert>
#include <unordered_map>
#include "Buffer.h"
#include "Device.h"
#include "Shader.h"
#include "Types/DescriptorData.h"
#include "Descriptor/DescriptorAllocator.h"
#include "VulkanCheckResult.h"
#include "Types/DeletionQueue.h"
#include "../Camera/PerspectiveCamera.h"
#include <imgui.h>
#include "Pipeline.h"
#include <Log.h>
namespace FooGame
{

#if 1
#define VERT_SHADER          "../../../Assets/Shaders/vert.spv"
#define FRAG_SHADER          "../../../Assets/Shaders/frag.spv"
#define DEFAULT_TEXTURE_PATH "../../../Assets/Textures/texture.jpg"
#else

#define VERT_SHADER          "../../Assets/Shaders/vert.spv"
#define FRAG_SHADER          "../../Assets/Shaders/frag.spv"
#define MODEL_PATH           "../../Assets/Model/viking_room.obj"
#define DEFAULT_TEXTURE_PATH "../../Assets/Textures/texture.jpg"
#endif

    struct MeshPushConstants
    {
            glm::vec4 data;
            glm::mat4 renderMatrix;
    };
    struct MeshDrawData
    {
            Buffer* VertexBuffer = nullptr;
            Buffer* IndexBuffer  = nullptr;
            Mesh* PtrMesh        = nullptr;
    };
    struct StaticMeshContainer
    {
            std::vector<std::unique_ptr<Buffer>> VertexBuffer;
            std::vector<std::unique_ptr<Buffer>> IndexBuffer;
            size_t TotalSize = 0;
            // todo some sort of data structure for keeping indexes offsets etc
    };

    struct RenderData
    {
            struct Resources
            {
                    std::unordered_map<uint32_t, MeshDrawData> MeshMap;
                    uint32_t FreeIndex = 0;
                    std::vector<Buffer*> UniformBuffers;
                    std::shared_ptr<Model> DefaultModel = nullptr;
                    DescriptorData descriptor;
                    vke::DescriptorAllocatorPool* DescriptorAllocatorPool;
                    DeletionQueue deletionQueue;
            };

            struct Api
            {
                    Pipeline GraphicsPipeline;
                    VkSampler TextureSampler;
                    std::shared_ptr<Texture2D> DefaultTexture;
            };
            Resources Res;
            FrameStatistics FrameData;
            Api api{};
    };
    static RenderData s_Data;
    static bool g_IsInitialized = false;
    void Renderer3D::Init()
    {
        assert(!g_IsInitialized && "Do not init renderer3d twice!");
        FOO_ENGINE_INFO("3D Renderer system initializing!");
        auto* device    = Api::GetDevice();
        g_IsInitialized = true;
        s_Data.Res.UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            BufferBuilder uBuffBuilder{};
            uBuffBuilder.SetUsage(BufferUsage::UNIFORM)
                .SetInitialSize(sizeof(UniformBufferObject))
                .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            s_Data.Res.UniformBuffers[i] = CreateDynamicBuffer(
                sizeof(UniformBufferObject), BufferUsage::UNIFORM);
        }
        s_Data.Res.deletionQueue.PushFunction(
            [&](VkDevice device)
            {
                for (auto& buffer : s_Data.Res.UniformBuffers)
                {
                    buffer->Release();
                }
                s_Data.Res.UniformBuffers.clear();
            });

        s_Data.Res.DescriptorAllocatorPool =
            vke::DescriptorAllocatorPool::Create(device->GetDevice());
        s_Data.Res.deletionQueue.PushFunction(
            [&](VkDevice device)
            { delete s_Data.Res.DescriptorAllocatorPool; });
        auto allocator = s_Data.Res.DescriptorAllocatorPool->GetAllocator();
        s_Data.Res.DescriptorAllocatorPool->SetPoolSizeMultiplier(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT);
        {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding         = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding         = 1;
            samplerLayoutBinding.descriptorCount = 1;
            samplerLayoutBinding.descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding,
                                                        samplerLayoutBinding};
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType =
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = ARRAY_COUNT(bindings);
            layoutInfo.pBindings    = bindings;

            VK_CALL(vkCreateDescriptorSetLayout(
                device->GetDevice(), &layoutInfo, nullptr,
                &s_Data.Res.descriptor.SetLayout));
            s_Data.Res.deletionQueue.PushFunction(
                [&](VkDevice device)
                {
                    vkDestroyDescriptorSetLayout(
                        device, s_Data.Res.descriptor.SetLayout, nullptr);
                });
        }
        {
            s_Data.api.DefaultTexture = LoadTexture(DEFAULT_TEXTURE_PATH);
            s_Data.Res.deletionQueue.PushFunction(
                [&](VkDevice device)
                {
                    DestroyImage(s_Data.api.DefaultTexture.get());
                    s_Data.api.DefaultTexture.reset();
                });
        }
        {
            PipelineInfo info{};
            Shader vert{VERT_SHADER, ShaderStage::VERTEX};
            Shader frag{FRAG_SHADER, ShaderStage::FRAGMENT};
            info.Shaders = std::vector<Shader*>{&vert, &frag};
            info.VertexAttributeDescriptons =
                Vertex::GetAttributeDescriptionList();
            info.VertexBindings      = {Vertex::GetBindingDescription()};
            info.LineWidth           = 2.0f;
            info.CullMode            = CullMode::FRONT;
            info.MultiSampling       = MultiSampling::LEVEL_1;
            info.DescriptorSetLayout = s_Data.Res.descriptor.SetLayout;
            info.pushConstantSize    = sizeof(MeshPushConstants);
            info.pushConstantCount   = 1;

            s_Data.api.GraphicsPipeline = CreateGraphicsPipeline(info);
            s_Data.Res.deletionQueue.PushFunction(
                [&](VkDevice device)
                {
                    vkDestroyPipelineLayout(
                        device, s_Data.api.GraphicsPipeline.pipelineLayout,
                        nullptr);
                    vkDestroyPipeline(Api::GetVkDevice(),
                                      s_Data.api.GraphicsPipeline.pipeline,
                                      nullptr);
                });
        }
    }
    void Renderer3D::SubmitModel(Model* model)
    {
        for (auto& mesh : model->GetMeshes())
        {
            int id = SubmitMesh(&mesh);
            model->PushId(id);
        }
    }
    void Renderer3D::BeginDraw()
    {
        s_Data.FrameData.DrawCall = 0;
    }
    FrameStatistics Renderer3D::GetStats()
    {
        return s_Data.FrameData;
    }
    void Renderer3D::EndDraw()
    {
        s_Data.Res.DescriptorAllocatorPool->Flip();
    }
    void Renderer3D::BeginScene(const PerspectiveCamera& camera)
    {
        UniformBufferObject ubd{};

        ubd.View       = camera.GetView();
        ubd.Projection = camera.GetProjection();
        UpdateUniformData(ubd);
    }

    void Renderer3D::EndScene()
    {
    }
    uint32_t Renderer3D::SubmitMesh(Mesh* mesh)
    {
        s_Data.Res.MeshMap[s_Data.Res.FreeIndex] = {
            CreateVertexBuffer(mesh->m_Vertices),
            CreateIndexBuffer(mesh->m_Indices), mesh};
        s_Data.Res.FreeIndex++;
        return s_Data.Res.FreeIndex - 1;
    }
    void Renderer3D::ClearBuffers()
    {
        for (auto& [index, data] : s_Data.Res.MeshMap)
        {
            data.VertexBuffer->Release();
            data.IndexBuffer->Release();
        }
    }

    void Renderer3D::DrawModel(uint32_t id, const glm::mat4& transform)
    {
        auto currentFrame = Backend::GetCurrentFrame();
        auto cmd          = Backend::GetCurrentCommandbuffer();
        auto extent       = Backend::GetSwapchainExtent();

        BindPipeline(cmd);

        auto& modelRes = s_Data.Res.MeshMap[id];

        Api::SetViewportAndScissors(cmd, extent.width, extent.height);
        VkBuffer vertexBuffers[] = {*modelRes.VertexBuffer->GetBuffer()};
        VkDeviceSize offsets[]   = {0};
        MeshPushConstants push{};
        push.renderMatrix = transform;
        auto allocator    = s_Data.Res.DescriptorAllocatorPool->GetAllocator();
        allocator.Allocate(modelRes.PtrMesh->GetLayout(),
                           *modelRes.PtrMesh->GetSet(currentFrame));
        vkCmdPushConstants(cmd, s_Data.api.GraphicsPipeline.pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(MeshPushConstants), &push);
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(cmd, *modelRes.IndexBuffer->GetBuffer(), 0,
                             VK_INDEX_TYPE_UINT32);
        BindDescriptorSets(cmd, *modelRes.PtrMesh, s_Data.api.GraphicsPipeline);
        vkCmdDrawIndexed(cmd, modelRes.PtrMesh->m_Indices.size(), 1, 0, 0, 0);
        s_Data.FrameData.DrawCall++;
    }
    void Renderer3D::DrawMesh(uint32_t id, const glm::mat4& transform,
                              const Texture2D& texture)
    {
        auto currentFrame = Backend::GetCurrentFrame();
        auto cmd          = Backend::GetCurrentCommandbuffer();

        BindPipeline(cmd);

        auto& modelRes = s_Data.Res.MeshMap[id];

        auto extent = Backend::GetSwapchainExtent();
        Api::SetViewportAndScissors(cmd, extent.width, extent.height);
        VkBuffer vertexBuffers[] = {*modelRes.VertexBuffer->GetBuffer()};
        VkDeviceSize offsets[]   = {0};
        MeshPushConstants push{};
        push.renderMatrix = transform;
        auto allocator    = s_Data.Res.DescriptorAllocatorPool->GetAllocator();
        allocator.Allocate(modelRes.PtrMesh->GetLayout(),
                           *modelRes.PtrMesh->GetSet(currentFrame));
        vkCmdPushConstants(cmd, s_Data.api.GraphicsPipeline.pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(MeshPushConstants), &push);
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(cmd, *modelRes.IndexBuffer->GetBuffer(), 0,
                             VK_INDEX_TYPE_UINT32);
        BindDescriptorSets(cmd, texture,
                           *modelRes.PtrMesh->GetSet(currentFrame),
                           s_Data.api.GraphicsPipeline);
        vkCmdDrawIndexed(cmd, modelRes.PtrMesh->m_Indices.size(), 1, 0, 0, 0);
        s_Data.FrameData.DrawCall++;
    }
    void Renderer3D::BindDescriptorSets(
        VkCommandBuffer cmd, const Texture2D& texture, VkDescriptorSet& set,
        Pipeline& pipeLine, VkPipelineBindPoint bindPoint, uint32_t firstSet,
        uint32_t dSetCount, uint32_t dynamicOffsetCount,
        uint32_t* dynamicOffsets)
    {
        auto currentFrame = Backend::GetCurrentFrame();
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer =
            *s_Data.Res.UniformBuffers[currentFrame]->GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrites[2] = {};
        descriptorWrites[0].sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet     = set;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo     = &bufferInfo;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = texture.ImageView;
        imageInfo.sampler     = texture.Sampler;

        descriptorWrites[1].sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet     = set;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo      = &imageInfo;

        vkUpdateDescriptorSets(Api::GetVkDevice(),
                               ARRAY_COUNT(descriptorWrites), descriptorWrites,
                               0, nullptr);
        VkDescriptorSet sets[] = {set};
        vkCmdBindDescriptorSets(cmd, bindPoint, pipeLine.pipelineLayout, 0, 1,
                                sets, 0, nullptr);
    }
    void Renderer3D::BindDescriptorSets(VkCommandBuffer cmd, Mesh& mesh,
                                        Pipeline& pipeLine,
                                        VkPipelineBindPoint bindPoint,
                                        uint32_t firstSet, uint32_t dSetCount,
                                        uint32_t dynamicOffsetCount,
                                        uint32_t* dynamicOffsets)
    {
        auto currentFrame = Backend::GetCurrentFrame();
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer =
            *s_Data.Res.UniformBuffers[currentFrame]->GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrites[2] = {};
        descriptorWrites[0].sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet     = *mesh.GetSet(currentFrame);
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo     = &bufferInfo;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = mesh.m_Texture->ImageView;
        imageInfo.sampler     = mesh.m_Texture->Sampler;

        descriptorWrites[1].sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet     = *mesh.GetSet(currentFrame);
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo      = &imageInfo;

        vkUpdateDescriptorSets(Api::GetVkDevice(),
                               ARRAY_COUNT(descriptorWrites), descriptorWrites,
                               0, nullptr);
        vkCmdBindDescriptorSets(cmd, bindPoint, pipeLine.pipelineLayout, 0, 1,
                                mesh.GetSet(currentFrame), 0, nullptr);
    }
    void Renderer3D::BindPipeline(VkCommandBuffer cmd)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          s_Data.api.GraphicsPipeline.pipeline);
    }

    void Renderer3D::Shutdown()
    {
        auto device = Api::GetVkDevice();
        s_Data.Res.deletionQueue.Flush(device);
        for (auto& [index, data] : s_Data.Res.MeshMap)
        {
            data.VertexBuffer->Release();
            data.IndexBuffer->Release();
        }
    }
    void Renderer3D::UpdateUniformData(UniformBufferObject ubd)
    {
        s_Data.Res.UniformBuffers[Backend::GetCurrentFrame()]->SetData(
            sizeof(ubd), &ubd);
    }

}  // namespace FooGame
