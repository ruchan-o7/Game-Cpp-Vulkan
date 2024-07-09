#include "Renderer3D.h"
#include "Backend.h"
#include <Log.h>
#include <imgui.h>
#include "Shader.h"
#include "Types/DeletionQueue.h"
#include "../Core/VulkanPipeline.h"
#include "../Core/RenderDevice.h"
#include "../Core/VulkanBuffer.h"
#include "../Camera/PerspectiveCamera.h"
#include "../Camera/Camera.h"
#include "../../Core/AssetManager.h"
#include "../../Engine/Core/VulkanTexture.h"
#include "../../Engine/Engine/Types/GraphicTypes.h"
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
            Unique<VulkanBuffer> VertexBuffer = nullptr;
            Unique<VulkanBuffer> IndexBuffer  = nullptr;
            Model* PtrModel                   = nullptr;
    };

    struct RenderData
    {
            struct Resources
            {
                    Hashmap<u32, MeshDrawData2> MeshMap2;
                    u32 FreeIndex              = 0;
                    Shared<Model> DefaultModel = nullptr;
                    DeletionQueue deletionQueue;
                    bool Exists(UUID id)
                    {
                        if (MeshMap2.find(id) != MeshMap2.end())
                        {
                            return true;
                        }
                        return false;
                    }
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
            std::vector<Unique<VulkanBuffer>> uniformBuffers{2};
            Unique<VulkanPipeline> pGraphicPipeline;
            VkDescriptorSet descriptorSets[3];
    };
    RendererContext rContext{};
    static RenderData s_Data;
    static bool g_IsInitialized = false;
    VulkanPipeline* Renderer3D::GetPipeline()
    {
        return rContext.pGraphicPipeline.get();
    }
    void Renderer3D::Init(class RenderDevice* pRenderDevice)
    {
        assert(!g_IsInitialized && "Do not init renderer3d twice!");
        FOO_ENGINE_INFO("3D Renderer system initializing!");
        auto* device    = pRenderDevice->GetVkDevice();
        g_IsInitialized = true;

        for (u32 i = 0; i < 2; i++)
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

    void Renderer3D::SubmitModel(UUID id)
    {
        auto modelAsset = AssetManager::GetModelAsset(id);
        if (modelAsset == nullptr)
        {
            FOO_ENGINE_WARN("Submitted model asset is null");
            return;
        }
        auto model = modelAsset->Asset;

        size_t vertexSize = sizeof(model->Vertices[0]) * model->Vertices.size();
        VulkanBuffer::BuffDesc vInfo{};
        vInfo.pRenderDevice   = Backend::GetRenderDevice();
        vInfo.BufferData.Data = model->Vertices.data();
        vInfo.BufferData.Size = vertexSize;
        vInfo.Name            = "Model vb: " + model->Name;
        auto vb               = VulkanBuffer::CreateVertexBuffer(vInfo);

        Unique<VulkanBuffer> ib;
        if (model->Indices.size() != 0)
        {
            size_t indicesSize = sizeof(model->Indices[0]) * model->Indices.size();
            VulkanBuffer::BuffDesc iInfo{};
            iInfo.pRenderDevice   = Backend::GetRenderDevice();
            iInfo.BufferData.Data = model->Indices.data();
            iInfo.BufferData.Size = indicesSize;
            iInfo.Name            = "Mesh ib" + model->Name;
            ib                    = VulkanBuffer::CreateIndexBuffer(iInfo);
        }

        s_Data.Res.MeshMap2[s_Data.Res.FreeIndex] = {std::move(vb), std::move(ib), model.get()};

        model->RenderId = s_Data.Res.FreeIndex;
        s_Data.Res.FreeIndex++;
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
        ubd.View       = camera.View;
        ubd.Projection = camera.Perspective;
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
        scissor.extent = {static_cast<u32>(viewport.width), static_cast<u32>(viewport.height)};
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

    void Renderer3D::DrawModel(UUID id, const glm::mat4& transform)
    {
        auto* asset = AssetManager::GetModelAsset(id);
        if (!asset)
        {
            return;
        }
        if (asset->Status != Asset::AssetStatus::READY)
        {
            return;
        }

        if (asset->Asset->Name.empty())
        {
            return;
        }

        auto currentFrame = Backend::GetCurrentFrame();
        auto cmd          = Backend::GetCurrentCommandbuffer();

        auto allocator = Backend::GetAllocatorHandle();
        auto exists    = s_Data.Res.Exists(asset->Asset->RenderId);
        if (!exists)
        {
            return;
        }

        auto& modelRes = s_Data.Res.MeshMap2[asset->Asset->RenderId];

        for (const auto& mesh : asset->Asset->Meshes)
        {
            for (const auto& primitive : mesh.Primitives)
            {
                MeshPushConstants push{};
                push.renderMatrix = transform * mesh.Transform;
                Backend::PushConstant(rContext.pGraphicPipeline->GetLayout(),
                                      VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants),
                                      &push);
                const auto* materialAsset = AssetManager::GetMaterialAsset(primitive.MaterialId);
                if (materialAsset == nullptr)
                {
                    materialAsset = AssetManager::GetDefaultMaterial();
                }
                auto material = materialAsset->Asset;

                AssetTextureC* baseColorTextureAsset =
                    AssetManager::GetTextureAsset(material->BaseColorTexture.id);

                auto baseColorTexture = baseColorTextureAsset->Asset;

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
                Backend::BindGraphicPipelineDescriptorSets(rContext.pGraphicPipeline->GetLayout(),
                                                           0, 1, sets, 0, nullptr);

                VkBuffer vertexBuffers[] = {modelRes.VertexBuffer->GetBuffer()};
                VkDeviceSize offsets[]   = {0};

                Backend::BindVertexBuffers(0, 1, vertexBuffers, offsets);

                if (modelRes.IndexBuffer)
                {
                    Backend::BindIndexBuffers(modelRes.IndexBuffer->GetBuffer(), 0,
                                              VK_INDEX_TYPE_UINT32);
                    Backend::DrawIndexed(primitive.IndexCount, 1, primitive.FirstIndex, 0, 0);
                }
                else
                {
                    Backend::Draw(modelRes.PtrModel->Vertices.size(), 1, primitive.FirstIndex, 0);
                }
                s_Data.FrameData.DrawCall++;
                s_Data.FrameData.VertexCount += modelRes.PtrModel->Vertices.size();
                s_Data.FrameData.IndexCount  += modelRes.PtrModel->Indices.size();
            }
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
