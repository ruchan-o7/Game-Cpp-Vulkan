#include "Renderer3D.h"
#include "Api.h"
#include "Backend.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "Shader.h"
#include "../Core/VulkanPipeline.h"
#include "Types/DescriptorData.h"
#include "VulkanCheckResult.h"
#include "Types/DeletionQueue.h"
#include "../Core/RenderDevice.h"
#include "../Core/VulkanBuffer.h"
#include "../Camera/PerspectiveCamera.h"
#include "../Camera/Camera.h"
#include <imgui.h>
#include "Pipeline.h"
#include "Descriptor/DescriptorAllocator.h"
#include <Log.h>
namespace FooGame
{
#define VERT_SHADER          "Assets/Shaders/vert.spv"
#define FRAG_SHADER          "Assets/Shaders/frag.spv"
#define MODEL_PATH           "Assets/Model/viking_room.obj"
#define DEFAULT_TEXTURE_PATH "Assets/Textures/texture.jpg"
    struct MeshPushConstants
    {
            glm::vec4 data;
            glm::mat4 renderMatrix;
    };
    // struct MeshDrawData
    // {
    //         Buffer* VertexBuffer = nullptr;
    //         Buffer* IndexBuffer  = nullptr;
    //         Mesh* PtrMesh        = nullptr;
    // };

    struct MeshDrawData2
    {
            std::unique_ptr<VulkanBuffer> VertexBuffer = nullptr;
            std::unique_ptr<VulkanBuffer> IndexBuffer  = nullptr;
            Mesh* PtrMesh                              = nullptr;
    };
    struct StaticMeshContainer
    {
            std::vector<std::unique_ptr<VulkanTexture>> VertexBuffer;
            std::vector<std::unique_ptr<VulkanTexture>> IndexBuffer;
            size_t TotalSize = 0;
            // todo some sort of data structure for keeping indexes offsets etc
    };

    struct RenderData
    {
            struct Resources
            {
                    // std::unordered_map<uint32_t, MeshDrawData> MeshMap;
                    std::unordered_map<uint32_t, MeshDrawData2> MeshMap2;
                    uint32_t FreeIndex                  = 0;
                    std::shared_ptr<Model> DefaultModel = nullptr;
                    DescriptorData descriptor;
                    vke::DescriptorAllocatorPool* DescriptorAllocatorPool;
                    DeletionQueue deletionQueue;
            };

            struct Api
            {
                    VkSampler TextureSampler;
                    // Texture2D DefaultTexture;  // Owner is Renderer3D
            };
            Resources Res;
            FrameStatistics FrameData;
            Api api{};
    };
    struct RendererContext
    {
            std::vector<std::unique_ptr<VulkanBuffer>> buffers{MAX_FRAMES_IN_FLIGHT};
            CommandPoolWrapper CommandPool;
            std::unique_ptr<VulkanPipeline> pGraphicPipeline;
    };
    RendererContext rContext{};
    static RenderData s_Data;
    static bool g_IsInitialized = false;
    void Renderer3D::Init(class RenderDevice* pRenderDevice)
    {
        assert(!g_IsInitialized && "Do not init renderer3d twice!");
        FOO_ENGINE_INFO("3D Renderer system initializing!");
        auto* device    = pRenderDevice->GetVkDevice();
        g_IsInitialized = true;

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VulkanBuffer::BuffDesc desc{};
            desc.pRenderDevice   = pRenderDevice;
            desc.Usage           = Vulkan::BUFFER_USAGE_UNIFORM;
            desc.MemoryFlag      = Vulkan::BUFFER_MEMORY_FLAG_CPU_VISIBLE;
            desc.Name            = "Uniform buffer";
            desc.BufferData.Data = {0};
            desc.BufferData.Size = sizeof(UniformBufferObject);

            rContext.buffers[i] = std::move(std::make_unique<VulkanBuffer>(desc));
            rContext.buffers[i]->MapMemory();
        }
        s_Data.Res.DescriptorAllocatorPool = vke::DescriptorAllocatorPool::Create(device);
        s_Data.Res.DescriptorAllocatorPool->SetPoolSizeMultiplier(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                  MAX_FRAMES_IN_FLIGHT);
        s_Data.Res.deletionQueue.PushFunction([&](VkDevice device)
                                              { delete s_Data.Res.DescriptorAllocatorPool; });
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = 0;
        rContext.CommandPool      = pRenderDevice->GetLogicalDevice()->CreateCommandPool(poolInfo);
        {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding            = 0;
            uboLayoutBinding.descriptorCount    = 1;
            uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding             = 1;
            samplerLayoutBinding.descriptorCount     = 1;
            samplerLayoutBinding.descriptorType      = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers  = nullptr;
            samplerLayoutBinding.stageFlags          = VK_SHADER_STAGE_FRAGMENT_BIT;
            VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding, samplerLayoutBinding};
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = ARRAY_COUNT(bindings);
            layoutInfo.pBindings    = bindings;

            VK_CALL(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                                &s_Data.Res.descriptor.SetLayout));
            s_Data.Res.deletionQueue.PushFunction(
                [&](VkDevice device) {
                    vkDestroyDescriptorSetLayout(device, s_Data.Res.descriptor.SetLayout, nullptr);
                });
        }
        {
            auto queue = pRenderDevice->GetGraphicsQueue();

            // s_Data.Res.deletionQueue.PushFunction(
            //     [&](VkDevice device) { AssetLoader::DestroyTexture(s_Data.api.DefaultTexture);
            //     });
        }
        {
            auto logicalDevice = pRenderDevice->GetLogicalDevice();
            Shader vert{
                {VERT_SHADER, ShaderStage::VERTEX, logicalDevice}
            };
            Shader frag{
                {FRAG_SHADER, ShaderStage::FRAGMENT, logicalDevice}
            };
            VulkanPipeline::CreateInfo ci{};
            ci.RenderPass = Backend::GetRenderPass();
            ci.ShaderStages.push_back(vert.CreateInfo());
            ci.ShaderStages.push_back(frag.CreateInfo());
            ci.PushConstantCount      = 1;
            ci.PushConstantSize       = sizeof(MeshPushConstants);
            ci.SetLayout              = s_Data.Res.descriptor.SetLayout;
            ci.SampleCount            = 1;
            ci.wpLogicalDevice        = pRenderDevice->GetLogicalDevice();
            ci.CullMode               = CULL_MODE_BACK;
            ci.VertexAttributes       = Vertex::GetAttributeDescriptionList();
            ci.VertexBindings         = {Vertex::GetBindingDescription()};
            rContext.pGraphicPipeline = std::make_unique<VulkanPipeline>(ci);
        }
    }

    VkCommandPool Backend::GetCommandPool()
    {
        return rContext.CommandPool;
    }
    void Renderer3D::SubmitModel(Model* model)
    {
        for (auto& mesh : model->GetMeshes())
        {
            SubmitMesh(&mesh);
        }
    }
    void Renderer3D::BeginDraw()
    {
    }
    FrameStatistics Renderer3D::GetStats()
    {
        return s_Data.FrameData;
    }
    void Renderer3D::EndDraw()
    {
        s_Data.Res.DescriptorAllocatorPool->Flip();

        s_Data.FrameData.DrawCall    = 0;
        s_Data.FrameData.VertexCount = 0;
        s_Data.FrameData.IndexCount  = 0;
    }
    void Renderer3D::BeginScene(const Camera& camera)
    {
        UniformBufferObject ubd{};
        ubd.View       = camera.matrices.View;
        ubd.Projection = camera.matrices.Perspective;
        UpdateUniformData(ubd);
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
    void Renderer3D::SubmitMesh(Mesh* mesh)
    {
        VulkanBuffer::BuffDesc vInfo{};
        vInfo.pRenderDevice   = Backend::GetRenderDevice();
        vInfo.BufferData.Data = mesh->m_Vertices.data();
        vInfo.BufferData.Size = mesh->m_Vertices.size();
        vInfo.Name            = "Mesh vb";
        auto vb               = VulkanBuffer::CreateVertexBuffer(vInfo);

        std::unique_ptr<VulkanBuffer> ib;
        if (mesh->m_Indices.size() != 0)
        {
            VulkanBuffer::BuffDesc iInfo{};
            iInfo.pRenderDevice   = Backend::GetRenderDevice();
            iInfo.BufferData.Data = mesh->m_Indices.data();
            iInfo.BufferData.Size = mesh->m_Indices.size();
            iInfo.Name            = "Mesh ib";
            ib                    = VulkanBuffer::CreateIndexBuffer(iInfo);
        }

        s_Data.Res.MeshMap2[s_Data.Res.FreeIndex] = {std::move(vb), std::move(ib), mesh};

        mesh->RenderId = s_Data.Res.FreeIndex;
        s_Data.Res.FreeIndex++;
    }
    void Renderer3D::ClearBuffers()
    {
        // for (auto& [index, data] : s_Data.Res.MeshMap)
        // {
        //     data.VertexBuffer->Release();
        //     if (data.IndexBuffer)
        //     {
        //         data.IndexBuffer->Release();
        //     }
        // }
        // s_Data.Res.MeshMap.clear();
    }

    void Renderer3D::DrawModel(Model* model, const glm::mat4& transform)
    {
        auto currentFrame = Backend::GetCurrentFrame();
        auto cmd          = Backend::GetCurrentCommandbuffer();
        auto extent       = Backend::GetSwapchainExtent();
        auto device       = Backend::GetRenderDevice()->GetVkDevice();

        BindPipeline(cmd);

        const auto& meshes = model->GetMeshes();
        for (uint32_t i = 0; i < meshes.size(); i++)
        {
            const auto& mesh = meshes[i];
            auto& modelRes   = s_Data.Res.MeshMap2[mesh.RenderId];

            VkViewport viewport{};
            viewport.x        = 0;
            viewport.y        = 0;
            viewport.width    = extent.width;
            viewport.height   = extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(cmd, 0, 1, &viewport);
            VkRect2D scissor{
                {                                    0,                                      0},
                {static_cast<uint32_t>(viewport.width), static_cast<uint32_t>(viewport.height)}
            };
            vkCmdSetScissor(cmd, 0, 1, &scissor);

            VkBuffer vertexBuffers[] = {modelRes.VertexBuffer->GetBuffer()};
            VkDeviceSize offsets[]   = {0};
            MeshPushConstants push{};
            push.renderMatrix = transform;
            auto allocator    = s_Data.Res.DescriptorAllocatorPool->GetAllocator();
            auto& currentSet  = *modelRes.PtrMesh->GetSet(currentFrame);
            allocator.Allocate(modelRes.PtrMesh->GetLayout(), currentSet);
            vkCmdPushConstants(cmd, rContext.pGraphicPipeline->GetLayout(),
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &push);
            {
                VkDescriptorBufferInfo descriptorBufferInfo{};
                descriptorBufferInfo.buffer = rContext.buffers[currentFrame]->GetBuffer();
                descriptorBufferInfo.offset = 0;
                descriptorBufferInfo.range  = sizeof(UniformBufferObject);
                std::vector<VkWriteDescriptorSet> descriptorWrites;
                VkWriteDescriptorSet uniformSet{};

                uniformSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                uniformSet.dstSet          = currentSet;
                uniformSet.dstBinding      = 0;
                uniformSet.dstArrayElement = 0;
                uniformSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                uniformSet.descriptorCount = 1;
                uniformSet.pBufferInfo     = &descriptorBufferInfo;
                descriptorWrites.push_back(uniformSet);
                for (auto& image : model->Textures)
                {
                    VkWriteDescriptorSet imageSet{};
                    imageSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    imageSet.dstSet          = currentSet;
                    imageSet.dstBinding      = 1;
                    imageSet.dstArrayElement = 0;
                    imageSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    imageSet.descriptorCount = 1;
                    imageSet.pImageInfo      = &image->DescriptorInfo;
                    descriptorWrites.push_back(imageSet);
                }
                vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0,
                                       nullptr);
                VkDescriptorSet sets[] = {currentSet};
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        rContext.pGraphicPipeline->GetLayout(), 0, 1, sets, 0,
                                        nullptr);
            }
            vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
            if (modelRes.IndexBuffer)
            {
                vkCmdBindIndexBuffer(cmd, modelRes.IndexBuffer->GetBuffer(), 0,
                                     VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(cmd, modelRes.PtrMesh->m_Indices.size(), 1, 0, 0, 0);
            }
            else
            {
                vkCmdDraw(cmd, mesh.m_Vertices.size(), 1, 0, 0);
            }
            // BindDescriptorSets(cmd, *modelRes.PtrMesh,
            //                    s_Data.api.GraphicsPipeline);
            s_Data.FrameData.DrawCall++;
            s_Data.FrameData.VertexCount += modelRes.PtrMesh->m_Vertices.size();
            s_Data.FrameData.IndexCount  += modelRes.PtrMesh->m_Indices.size();
        }
    }
    void Renderer3D::DrawModel(uint32_t id, const glm::mat4& transform)
    {
        auto currentFrame = Backend::GetCurrentFrame();
        auto cmd          = Backend::GetCurrentCommandbuffer();
        auto extent       = Backend::GetSwapchainExtent();

        BindPipeline(cmd);

        auto& modelRes = s_Data.Res.MeshMap2[id];

        Api::SetViewportAndScissors(cmd, extent.width, extent.height);
        VkBuffer vertexBuffers[] = {modelRes.VertexBuffer->GetBuffer()};
        VkDeviceSize offsets[]   = {0};
        MeshPushConstants push{};
        push.renderMatrix = transform;
        auto allocator    = s_Data.Res.DescriptorAllocatorPool->GetAllocator();
        allocator.Allocate(modelRes.PtrMesh->GetLayout(), *modelRes.PtrMesh->GetSet(currentFrame));
        vkCmdPushConstants(cmd, rContext.pGraphicPipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT,
                           0, sizeof(MeshPushConstants), &push);
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
        if (modelRes.IndexBuffer)
        {
            vkCmdBindIndexBuffer(cmd, modelRes.IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
        BindDescriptorSets(cmd, *modelRes.PtrMesh, rContext.pGraphicPipeline->GetLayout());
        vkCmdDrawIndexed(cmd, modelRes.PtrMesh->m_Indices.size(), 1, 0, 0, 0);
        s_Data.FrameData.DrawCall++;
        s_Data.FrameData.VertexCount += modelRes.PtrMesh->m_Vertices.size();
        s_Data.FrameData.IndexCount  += modelRes.PtrMesh->m_Indices.size();
    }
    // void Renderer3D::DrawMesh(uint32_t id, const glm::mat4& transform, const Texture2D& texture)
    // {
    //     auto currentFrame = Backend::GetCurrentFrame();
    //     auto cmd          = Backend::GetCurrentCommandbuffer();
    //
    //     BindPipeline(cmd);
    //
    //     auto& modelRes = s_Data.Res.MeshMap[id];
    //
    //     auto extent = Backend::GetSwapchainExtent();
    //     Api::SetViewportAndScissors(cmd, extent.width, extent.height);
    //     VkBuffer vertexBuffers[] = {*modelRes.VertexBuffer->GetBuffer()};
    //     VkDeviceSize offsets[]   = {0};
    //     MeshPushConstants push{};
    //     push.renderMatrix = transform;
    //     auto allocator    = s_Data.Res.DescriptorAllocatorPool->GetAllocator();
    //     allocator.Allocate(modelRes.PtrMesh->GetLayout(),
    //                        modelRes.PtrMesh->m_DescriptorSets[currentFrame]);
    //     vkCmdPushConstants(cmd, rContext.pGraphicPipeline->GetLayout(),
    //     VK_SHADER_STAGE_VERTEX_BIT,
    //                        0, sizeof(MeshPushConstants), &push);
    //     vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    //     if (modelRes.IndexBuffer)
    //     {
    //         vkCmdBindIndexBuffer(cmd, *modelRes.IndexBuffer->GetBuffer(), 0,
    //         VK_INDEX_TYPE_UINT32);
    //     }
    //     BindDescriptorSets(cmd, texture, *modelRes.PtrMesh->GetSet(currentFrame),
    //                        rContext.pGraphicPipeline->GetLayout());
    //     if (modelRes.IndexBuffer)
    //     {
    //         vkCmdDrawIndexed(cmd, modelRes.PtrMesh->m_Indices.size(), 1, 0, 0, 0);
    //     }
    //     else
    //     {
    //         vkCmdDraw(cmd, modelRes.PtrMesh->m_Vertices.size(), 1, 0, 0);
    //     }
    //     s_Data.FrameData.DrawCall++;
    //     s_Data.FrameData.VertexCount += modelRes.PtrMesh->m_Vertices.size();
    //     s_Data.FrameData.IndexCount  += modelRes.PtrMesh->m_Indices.size();
    // }
    // void Renderer3D::BindDescriptorSets(VkCommandBuffer cmd, const Texture2D& texture,
    //                                     VkDescriptorSet& set, VkPipelineLayout pipeline,
    //                                     VkPipelineBindPoint bindPoint, uint32_t firstSet,
    //                                     uint32_t dSetCount, uint32_t dynamicOffsetCount,
    //                                     uint32_t* dynamicOffsets)
    // {
    //     auto currentFrame = Backend::GetCurrentFrame();
    //     VkDescriptorBufferInfo bufferInfo{};
    //     bufferInfo.buffer = rContext.buffers[currentFrame]->GetBuffer();
    //     bufferInfo.offset = 0;
    //     bufferInfo.range  = sizeof(UniformBufferObject);
    //
    //     VkWriteDescriptorSet descriptorWrites[2] = {};
    //     descriptorWrites[0].sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //     descriptorWrites[0].dstSet               = set;
    //     descriptorWrites[0].dstBinding           = 0;
    //     descriptorWrites[0].dstArrayElement      = 0;
    //     descriptorWrites[0].descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //     descriptorWrites[0].descriptorCount      = 1;
    //     descriptorWrites[0].pBufferInfo          = &bufferInfo;
    //
    //     VkDescriptorImageInfo imageInfo{};
    //     imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //     imageInfo.imageView   = texture.ImageView;
    //     imageInfo.sampler     = texture.Sampler;
    //
    //     descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //     descriptorWrites[1].dstSet          = set;
    //     descriptorWrites[1].dstBinding      = 1;
    //     descriptorWrites[1].dstArrayElement = 0;
    //     descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //     descriptorWrites[1].descriptorCount = 1;
    //     descriptorWrites[1].pImageInfo      = &texture.Descriptor;
    //
    //     vkUpdateDescriptorSets(Api::GetVkDevice(), ARRAY_COUNT(descriptorWrites),
    //     descriptorWrites,
    //                            0, nullptr);
    //     VkDescriptorSet sets[] = {set};
    //     vkCmdBindDescriptorSets(cmd, bindPoint, pipeline, 0, 1, sets, 0, nullptr);
    // }
    void Renderer3D::BindDescriptorSets(VkCommandBuffer cmd, Mesh& mesh, VkPipelineLayout pipeline,
                                        VkPipelineBindPoint bindPoint, uint32_t firstSet,
                                        uint32_t dSetCount, uint32_t dynamicOffsetCount,
                                        uint32_t* dynamicOffsets)
    {
        auto currentFrame = Backend::GetCurrentFrame();
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = rContext.buffers[currentFrame]->GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrites[2] = {};
        descriptorWrites[0].sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet               = *mesh.GetSet(currentFrame);
        descriptorWrites[0].dstBinding           = 0;
        descriptorWrites[0].dstArrayElement      = 0;
        descriptorWrites[0].descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount      = 1;
        descriptorWrites[0].pBufferInfo          = &bufferInfo;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        // imageInfo.imageView   = mesh.m_Texture->ImageView;
        // imageInfo.sampler     = mesh.m_Texture->Sampler;

        descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet          = *mesh.GetSet(currentFrame);
        descriptorWrites[1].dstBinding      = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo      = &imageInfo;

        vkUpdateDescriptorSets(Api::GetVkDevice(), ARRAY_COUNT(descriptorWrites), descriptorWrites,
                               0, nullptr);
        vkCmdBindDescriptorSets(cmd, bindPoint, pipeline, 0, 1, mesh.GetSet(currentFrame), 0,
                                nullptr);
    }
    void Renderer3D::BindPipeline(VkCommandBuffer cmd)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          rContext.pGraphicPipeline->GetPipeline());
    }

    void Renderer3D::Shutdown()
    {
        auto device = Api::GetVkDevice();
        s_Data.Res.deletionQueue.Flush(device);
        for (auto& [index, data] : s_Data.Res.MeshMap2)
        {
            // data.VertexBuffer->Release();
            if (data.IndexBuffer)
            {
                // data.IndexBuffer->Release();
            }
        }
    }
    void Renderer3D::UpdateUniformData(UniformBufferObject& ubd)
    {
        rContext.buffers[Backend::GetCurrentFrame()]->UpdateData(&ubd, sizeof(ubd));
    }

}  // namespace FooGame
