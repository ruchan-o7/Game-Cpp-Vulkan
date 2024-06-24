#include "Renderer3D.h"
#include "Backend.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "Shader.h"
#include "../Core/VulkanPipeline.h"
#include "Types/DeletionQueue.h"
#include "../Core/RenderDevice.h"
#include "../Core/VulkanBuffer.h"
#include "../Camera/PerspectiveCamera.h"
#include "../Camera/Camera.h"
#include <imgui.h>
#include "src/Core/AssetManager.h"
#include "src/Engine/Core/VulkanTexture.h"
#include "src/Engine/Engine/Types/GraphicTypes.h"
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
                    std::unordered_map<uint32_t, MeshDrawData2> MeshMap2;
                    uint32_t FreeIndex                  = 0;
                    std::shared_ptr<Model> DefaultModel = nullptr;
                    DeletionQueue deletionQueue;
            };

            struct Api
            {
                    VkSampler TextureSampler;
            };
            Resources Res;
            FrameStatistics FrameData;
            Api api{};
    };
    struct RendererContext
    {
            std::vector<std::unique_ptr<VulkanBuffer>> uniformBuffers{2};
            std::unique_ptr<VulkanPipeline> pGraphicPipeline;
            VkDescriptorSet descriptorSets[3];
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

        for (uint32_t i = 0; i < 2; i++)
        {
            VulkanBuffer::BuffDesc desc{};
            desc.pRenderDevice   = pRenderDevice;
            desc.Usage           = Vulkan::BUFFER_USAGE_UNIFORM;
            desc.MemoryFlag      = Vulkan::BUFFER_MEMORY_FLAG_CPU_VISIBLE;
            desc.Name            = "Uniform buffer";
            desc.BufferData.Data = {0};
            desc.BufferData.Size = sizeof(UniformBufferObject);

            rContext.uniformBuffers[i] = std::move(std::make_unique<VulkanBuffer>(desc));
            rContext.uniformBuffers[i]->MapMemory();
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
            ci.SampleCount            = 1;
            ci.wpLogicalDevice        = pRenderDevice->GetLogicalDevice();
            ci.CullMode               = CULL_MODE_BACK;
            ci.VertexAttributes       = Vertex::GetAttributeDescriptionList();
            ci.VertexBindings         = {Vertex::GetBindingDescription()};
            rContext.pGraphicPipeline = std::make_unique<VulkanPipeline>(ci);
        }
    }

    void Renderer3D::SubmitModel(const std::string& name)
    {
        auto model = AssetManager::GetModel(name);
        if (model == nullptr)
        {
            FOO_ENGINE_WARN("Submitted mesh is null");
            return;
        }
        SubmitModel(model.get());
    }
    void Renderer3D::SubmitModel(std::shared_ptr<Model> model)
    {
        for (auto& mesh : model->GetMeshes())
        {
            SubmitMesh(&mesh);
        }
    }
    void Renderer3D::SubmitModel(Model* model)
    {
        for (auto& mesh : model->GetMeshes())
        {
            SubmitMesh(&mesh);
        }
    }
    FrameStatistics Renderer3D::GetStats()
    {
        return s_Data.FrameData;
    }
    void Renderer3D::EndDraw()
    {
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

        auto extent = Backend::GetSwapchainExtent();
        auto cmd    = Backend::GetCurrentCommandbuffer();
        BindPipeline(cmd);

        VkViewport viewport{};
        viewport.x        = 0;
        viewport.y        = 0;
        viewport.width    = extent.width;
        viewport.height   = extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        Backend::SetViewport(viewport);

        VkRect2D scissor{};
        scissor.extent = {static_cast<uint32_t>(viewport.width),
                          static_cast<uint32_t>(viewport.height)};
        scissor.offset = {0, 0};
        Backend::SetScissor(scissor);
    }
    void Renderer3D::BeginScene(const PerspectiveCamera& camera)
    {
        UniformBufferObject ubd{};

        ubd.View       = camera.GetView();
        ubd.Projection = camera.GetProjection();
        UpdateUniformData(ubd);
    }
    void Renderer3D::BeginScene(const glm::mat4& view, const glm::mat4& projection)
    {
        UniformBufferObject ubd{};
        ubd.View       = view;
        ubd.Projection = projection;
        UpdateUniformData(ubd);
    }

    void Renderer3D::SubmitMesh(Mesh& mesh)
    {
        size_t vertexSize = sizeof(mesh.m_Vertices[0]) * mesh.m_Vertices.size();
        VulkanBuffer::BuffDesc vInfo{};
        vInfo.pRenderDevice   = Backend::GetRenderDevice();
        vInfo.BufferData.Data = mesh.m_Vertices.data();
        vInfo.BufferData.Size = vertexSize;
        vInfo.Name            = "Mesh vb";
        auto vb               = VulkanBuffer::CreateVertexBuffer(vInfo);

        std::unique_ptr<VulkanBuffer> ib;
        if (mesh.m_Indices.size() != 0)
        {
            size_t indicesSize = sizeof(mesh.m_Indices[0]) * mesh.m_Indices.size();
            VulkanBuffer::BuffDesc iInfo{};
            iInfo.pRenderDevice   = Backend::GetRenderDevice();
            iInfo.BufferData.Data = mesh.m_Indices.data();
            iInfo.BufferData.Size = indicesSize;
            iInfo.Name            = "Mesh ib";
            ib                    = VulkanBuffer::CreateIndexBuffer(iInfo);
        }

        s_Data.Res.MeshMap2[s_Data.Res.FreeIndex] = {std::move(vb), std::move(ib), &mesh};

        mesh.RenderId = s_Data.Res.FreeIndex;
        s_Data.Res.FreeIndex++;
    }
    void Renderer3D::SubmitMesh(Mesh* mesh)
    {
        size_t vertexSize = sizeof(mesh->m_Vertices[0]) * mesh->m_Vertices.size();
        VulkanBuffer::BuffDesc vInfo{};
        vInfo.pRenderDevice   = Backend::GetRenderDevice();
        vInfo.BufferData.Data = mesh->m_Vertices.data();
        vInfo.BufferData.Size = vertexSize;
        vInfo.Name            = "Mesh vb: " + mesh->Name;
        auto vb               = VulkanBuffer::CreateVertexBuffer(vInfo);

        std::unique_ptr<VulkanBuffer> ib;
        if (mesh->m_Indices.size() != 0)
        {
            size_t indicesSize = sizeof(mesh->m_Indices[0]) * mesh->m_Indices.size();
            VulkanBuffer::BuffDesc iInfo{};
            iInfo.pRenderDevice   = Backend::GetRenderDevice();
            iInfo.BufferData.Data = mesh->m_Indices.data();
            iInfo.BufferData.Size = indicesSize;
            iInfo.Name            = "Mesh ib: " + mesh->Name;
            ib                    = VulkanBuffer::CreateIndexBuffer(iInfo);
        }

        s_Data.Res.MeshMap2[s_Data.Res.FreeIndex] = {std::move(vb), std::move(ib), mesh};

        mesh->RenderId = s_Data.Res.FreeIndex;
        s_Data.Res.FreeIndex++;
    }
    void Renderer3D::ClearBuffers()
    {
        for (auto& [id, data] : s_Data.Res.MeshMap2)
        {
            if (data.IndexBuffer)
            {
                data.IndexBuffer.reset();
            }
            data.VertexBuffer.reset();
        }
        s_Data.Res.MeshMap2.clear();
    }

    void Renderer3D::DrawMesh(const std::string& name, glm::mat4& transform)
    {
        if (name.empty())
        {
            FOO_ENGINE_WARN("Empty name string provided");
            return;
        }
        auto& asset = AssetManager::GetMeshAsset(name);
        if (asset.Status != AssetStatus::READY)
        {
            return;
        }

        if (asset.Asset->Name.empty())
        {
            return;
        }

        auto currentFrame = Backend::GetCurrentFrame();
        auto cmd          = Backend::GetCurrentCommandbuffer();
        auto extent       = Backend::GetSwapchainExtent();

        MeshPushConstants push{};
        push.renderMatrix = transform;
        Backend::PushConstant(rContext.pGraphicPipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                              sizeof(MeshPushConstants), &push);
        auto allocator = Backend::GetAllocatorHandle();
        auto& modelRes = s_Data.Res.MeshMap2[asset.Asset->RenderId];
        if (!modelRes.PtrMesh)
        {
            return;
        }
        const auto* material = AssetManager::GetMaterial(asset.Asset->M3Name);
        if (material == nullptr)
        {
            material = AssetManager::GetDefaultMaterial();
        }
        std::shared_ptr<VulkanTexture> baseColorTexture;
        if (material->BaseColorTexture.Name.empty())
        {
            baseColorTexture = AssetManager::GetDefaultTexture();
        }
        else
        {
            baseColorTexture = AssetManager::GetTexture(material->BaseColorTexture.Name);
        }

        auto& currentSet = rContext.descriptorSets[currentFrame];
        allocator.Allocate(rContext.pGraphicPipeline->GetDescriptorSetLayout(), currentSet);

        VkDescriptorBufferInfo descriptorBufferInfo{};
        descriptorBufferInfo.buffer = rContext.uniformBuffers[currentFrame]->GetBuffer();
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

        VkWriteDescriptorSet albedoSet{};
        albedoSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        albedoSet.dstSet          = currentSet;
        albedoSet.dstBinding      = 1;
        albedoSet.dstArrayElement = 0;
        albedoSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        albedoSet.descriptorCount = 1;
        albedoSet.pImageInfo      = &baseColorTexture->DescriptorInfo;
        descriptorWrites.push_back(albedoSet);

        Backend::UpdateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
        VkDescriptorSet sets[] = {currentSet};
        Backend::BindGraphicPipelineDescriptorSets(rContext.pGraphicPipeline->GetLayout(), 0, 1,
                                                   sets, 0, nullptr);

        VkBuffer vertexBuffers[] = {modelRes.VertexBuffer->GetBuffer()};
        VkDeviceSize offsets[]   = {0};

        Backend::BindVertexBuffers(0, 1, vertexBuffers, offsets);
        if (modelRes.IndexBuffer)
        {
            Backend::BindIndexBuffers(modelRes.IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
            Backend::DrawIndexed(modelRes.PtrMesh->m_Indices.size(), 1, 0, 0, 0);
        }
        else
        {
            Backend::Draw(asset.Asset->m_Vertices.size(), 1, 0, 0);
        }
        s_Data.FrameData.DrawCall++;
        s_Data.FrameData.VertexCount += modelRes.PtrMesh->m_Vertices.size();
        s_Data.FrameData.IndexCount  += modelRes.PtrMesh->m_Indices.size();
    }
    void Renderer3D::DrawModel(const std::string& name, const glm::mat4& transform)
    {
        if (name.empty() || name == "Empty Component")
        {
            return;
        }
        auto& asset = AssetManager::GetModelAsset(name);
        if (asset.Status != AssetStatus::READY)
        {
            return;
        }

        if (asset.Asset->Name.empty())
        {
            return;
        }

        auto currentFrame = Backend::GetCurrentFrame();
        auto cmd          = Backend::GetCurrentCommandbuffer();

        MeshPushConstants push{};
        push.renderMatrix = transform;
        Backend::PushConstant(rContext.pGraphicPipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                              sizeof(MeshPushConstants), &push);
        auto allocator = Backend::GetAllocatorHandle();
        for (const auto& mesh : asset.Asset->m_Meshes)
        {
            auto& modelRes = s_Data.Res.MeshMap2[mesh.RenderId];
            if (!modelRes.PtrMesh)
            {
                continue;
            }
            const auto* material = AssetManager::GetMaterial(mesh.M3Name);
            if (material == nullptr)
            {
                material = AssetManager::GetDefaultMaterial();
            }
            std::shared_ptr<VulkanTexture> baseColorTexture;
            if (material->BaseColorTexture.Name.empty())
            {
                baseColorTexture = AssetManager::GetDefaultTexture();
            }
            else
            {
                baseColorTexture = AssetManager::GetTexture(material->BaseColorTexture.Name);
            }

            auto& currentSet = rContext.descriptorSets[currentFrame];
            allocator.Allocate(rContext.pGraphicPipeline->GetDescriptorSetLayout(), currentSet);

            VkDescriptorBufferInfo descriptorBufferInfo{};
            descriptorBufferInfo.buffer = rContext.uniformBuffers[currentFrame]->GetBuffer();
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

            VkWriteDescriptorSet albedoSet{};
            albedoSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            albedoSet.dstSet          = currentSet;
            albedoSet.dstBinding      = 1;
            albedoSet.dstArrayElement = 0;
            albedoSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            albedoSet.descriptorCount = 1;
            albedoSet.pImageInfo      = &baseColorTexture->DescriptorInfo;
            descriptorWrites.push_back(albedoSet);

            Backend::UpdateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0,
                                          nullptr);
            VkDescriptorSet sets[] = {currentSet};
            Backend::BindGraphicPipelineDescriptorSets(rContext.pGraphicPipeline->GetLayout(), 0, 1,
                                                       sets, 0, nullptr);

            VkBuffer vertexBuffers[] = {modelRes.VertexBuffer->GetBuffer()};
            VkDeviceSize offsets[]   = {0};

            Backend::BindVertexBuffers(0, 1, vertexBuffers, offsets);
            if (modelRes.IndexBuffer)
            {
                Backend::BindIndexBuffers(modelRes.IndexBuffer->GetBuffer(), 0,
                                          VK_INDEX_TYPE_UINT32);
                Backend::DrawIndexed(modelRes.PtrMesh->m_Indices.size(), 1, 0, 0, 0);
            }
            else
            {
                Backend::Draw(mesh.m_Vertices.size(), 1, 0, 0);
            }
            s_Data.FrameData.DrawCall++;
            s_Data.FrameData.VertexCount += modelRes.PtrMesh->m_Vertices.size();
            s_Data.FrameData.IndexCount  += modelRes.PtrMesh->m_Indices.size();
        }
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
            Backend::SetViewport(viewport);
            VkRect2D scissor{
                {                                    0,                                      0},
                {static_cast<uint32_t>(viewport.width), static_cast<uint32_t>(viewport.height)}
            };
            Backend::SetScissor(scissor);

            VkBuffer vertexBuffers[] = {modelRes.VertexBuffer->GetBuffer()};
            VkDeviceSize offsets[]   = {0};
            MeshPushConstants push{};
            push.renderMatrix = transform;
            auto allocator    = Backend::GetAllocatorHandle();
            auto& currentSet  = rContext.descriptorSets[currentFrame];
            allocator.Allocate(rContext.pGraphicPipeline->GetDescriptorSetLayout(), currentSet);
            Backend::PushConstant(rContext.pGraphicPipeline->GetLayout(),
                                  VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &push);
            {
                VkDescriptorBufferInfo descriptorBufferInfo{};
                descriptorBufferInfo.buffer = rContext.uniformBuffers[currentFrame]->GetBuffer();
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
                // for (auto& image : model->Textures)
                // {
                //     VkWriteDescriptorSet imageSet{};
                //     imageSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                //     imageSet.dstSet          = currentSet;
                //     imageSet.dstBinding      = 1;
                //     imageSet.dstArrayElement = 0;
                //     imageSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                //     imageSet.descriptorCount = 1;
                //     imageSet.pImageInfo      = &image->DescriptorInfo;
                //     descriptorWrites.push_back(imageSet);
                // }
                Backend::UpdateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0,
                                              nullptr);
                VkDescriptorSet sets[] = {currentSet};
                Backend::BindGraphicPipelineDescriptorSets(rContext.pGraphicPipeline->GetLayout(),
                                                           0, 1, sets, 0, nullptr);
            }
            Backend::BindVertexBuffers(0, 1, vertexBuffers, offsets);
            if (modelRes.IndexBuffer)
            {
                Backend::BindIndexBuffers(modelRes.IndexBuffer->GetBuffer(), 0,
                                          VK_INDEX_TYPE_UINT32);
                Backend::DrawIndexed(modelRes.PtrMesh->m_Indices.size(), 1, 0, 0, 0);
            }
            else
            {
                Backend::Draw(mesh.m_Vertices.size(), 1, 0, 0);
            }
            s_Data.FrameData.DrawCall++;
            s_Data.FrameData.VertexCount += modelRes.PtrMesh->m_Vertices.size();
            s_Data.FrameData.IndexCount  += modelRes.PtrMesh->m_Indices.size();
        }
    }
    void Renderer3D::BindPipeline(VkCommandBuffer cmd)
    {
        Backend::BindGraphicPipeline(rContext.pGraphicPipeline->GetPipeline());
    }

    void Renderer3D::Shutdown()
    {
        auto device = Backend::GetRenderDevice()->GetVkDevice();  // Api::GetVkDevice();
        s_Data.Res.deletionQueue.Flush(device);
        for (auto& [index, data] : s_Data.Res.MeshMap2)
        {
            data.VertexBuffer.reset();
            if (data.IndexBuffer)
            {
                data.IndexBuffer.reset();
            }
        }
        rContext.pGraphicPipeline.reset();
        s_Data.Res.MeshMap2.clear();
    }
    void Renderer3D::UpdateUniformData(UniformBufferObject& ubd)
    {
        rContext.uniformBuffers[Backend::GetCurrentFrame()]->UpdateData(&ubd, sizeof(ubd));
    }

}  // namespace FooGame
