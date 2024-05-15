#include "Renderer3D.h"
#include "Api.h"
#include "Buffer.h"
#include "../Backend/Vertex.h"
#include "../Core/Base.h"
#include "../Core/Engine.h"
#include "../Graphics/Model.h"
#include "../Graphics/Pipeline.h"
#include "../Graphics/Texture2D.h"
#include "Core/Backend/VulkanCheckResult.h"
#include "Core/Core/PerspectiveCamera.h"
#include "Shader.h"
#include "../../Core/Graphics/Types/DescriptorData.h"
#include "vulkan/vulkan_core.h"
#include <cassert>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <imgui.h>
namespace FooGame
{
#if 1
#define VERT_SHADER "../../../Shaders/vert.spv"
#define FRAG_SHADER "../../../Shaders/frag.spv"
#else
#define VERT_SHADER "../../Shaders/vert.spv"
#define FRAG_SHADER "../../Shaders/frag.spv"
#define MODEL_PATH  "../../Assets/Model/viking_room.obj"

#endif

    struct MeshPushConstants
    {
            glm::vec4 data;
            glm::mat4 renderMatrix;
    };

    struct RenderData
    {
            struct Resources
            {
                    Buffer* VertexBuffer = nullptr;
                    Buffer* IndexBuffer  = nullptr;
                    List<Buffer*> UniformBuffers;
                    Shared<Model> DefaultModel = nullptr;
                    DescriptorData descriptor;
            };
            struct FrameData
            {
                    u32 DrawCall = 0;
            };
            Resources Res;
            FrameData FrameData;

            struct Api
            {
                    Pipeline GraphicsPipeline;
                    VkSampler TextureSampler;
                    Shared<Texture2D> DefaultTexture;
            };
            Api api{};
    };
    static RenderData s_Data;
    static bool g_IsInitialized = false;
    void Renderer3D::Init()
    {
        assert(!g_IsInitialized && "Do not init renderer3d twice!");
        auto* device    = Api::GetDevice();
        g_IsInitialized = true;
        s_Data.Res.UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            BufferBuilder uBuffBuilder{};
            uBuffBuilder.SetUsage(BufferUsage::UNIFORM)
                .SetInitialSize(sizeof(UniformBufferObject))
                .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            s_Data.Res.UniformBuffers[i] = CreateDynamicBuffer(
                sizeof(UniformBufferObject), BufferUsage::UNIFORM);
        }
        {
            VkDescriptorPoolSize poolSizes[1] = {};
            poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = ARRAY_COUNT(poolSizes);
            poolInfo.pPoolSizes    = poolSizes;
            poolInfo.maxSets       = MAX_FRAMES_IN_FLIGHT;
            VK_CALL(vkCreateDescriptorPool(device->GetDevice(), &poolInfo,
                                           nullptr,
                                           &s_Data.Res.descriptor.pool));
        }
        {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding         = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
            VkDescriptorSetLayoutBinding bindings[1] = {uboLayoutBinding};
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType =
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = ARRAY_COUNT(bindings);
            layoutInfo.pBindings    = bindings;

            VK_CALL(vkCreateDescriptorSetLayout(
                device->GetDevice(), &layoutInfo, nullptr,
                &s_Data.Res.descriptor.SetLayout));
            List<VkDescriptorSetLayout> layouts(
                MAX_FRAMES_IN_FLIGHT, s_Data.Res.descriptor.SetLayout);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool     = s_Data.Res.descriptor.pool;
            allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
            allocInfo.pSetLayouts        = layouts.data();

            VK_CALL(vkAllocateDescriptorSets(device->GetDevice(), &allocInfo,
                                             s_Data.Res.descriptor.Sets));
        }
        {
            for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = *s_Data.Res.UniformBuffers[i]->GetBuffer();
                bufferInfo.offset = 0;
                bufferInfo.range  = sizeof(UniformBufferObject);

                VkWriteDescriptorSet descriptorWrites[1] = {};
                descriptorWrites[0].sType =
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet     = s_Data.Res.descriptor.Sets[i];
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType =
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pBufferInfo     = &bufferInfo;

                vkUpdateDescriptorSets(device->GetDevice(),
                                       ARRAY_COUNT(descriptorWrites),
                                       descriptorWrites, 0, nullptr);
            }
        }
        {
            PipelineInfo info{};
            Shader vert{VERT_SHADER, ShaderStage::VERTEX};
            Shader frag{FRAG_SHADER, ShaderStage::FRAGMENT};
            info.Shaders = List<Shader*>{&vert, &frag};
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
        }
    }
    void Renderer3D::SubmitModel(const Shared<Model>& model)
    {
        if (s_Data.Res.VertexBuffer)
        {
            s_Data.Res.VertexBuffer->Release();
        }
        if (s_Data.Res.IndexBuffer)
        {
            s_Data.Res.IndexBuffer->Release();
        }
        s_Data.Res.VertexBuffer =
            CreateVertexBuffer(model->GetMeshes()[0].m_Vertices);
        s_Data.Res.IndexBuffer =
            CreateIndexBuffer(model->GetMeshes()[0].m_Indices);
    }
    void Renderer3D::BeginDraw()
    {
    }
    void Renderer3D::EndDraw()
    {
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
        Flush();
    }
    void Renderer3D::Flush()
    {
    }
    void Renderer3D::DrawModel(const Shared<Model>& model)
    {
        auto currentFrame = Engine::GetCurrentFrame();
        auto cmd          = Engine::GetCurrentCommandbuffer();
        auto extent       = Engine::GetSwapchainExtent();

        BindPipeline(cmd);

        Api::SetViewportAndScissors(cmd, extent.width, extent.height);
        VkBuffer vertexBuffers[] = {*s_Data.Res.VertexBuffer->GetBuffer()};
        VkDeviceSize offsets[]   = {0};
        MeshPushConstants push{};
        ImGui::Begin("Model position");
        float modelPos[3] = {model->Position.x, model->Position.y,
                             model->Position.z};
        ImGui::SliderFloat3("model pos", modelPos, -3.0f, 3.0f);
        model->Position.x = modelPos[0];
        model->Position.y = modelPos[1];
        model->Position.z = modelPos[2];

        ImGui::End();
        push.renderMatrix = glm::translate(glm::mat4(1.0f), model->Position);
        vkCmdPushConstants(cmd, s_Data.api.GraphicsPipeline.pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(MeshPushConstants), &push);
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(cmd, *s_Data.Res.IndexBuffer->GetBuffer(), 0,
                             VK_INDEX_TYPE_UINT32);
        BindDescriptorSets(cmd, s_Data.api.GraphicsPipeline);
        vkCmdDrawIndexed(cmd, model->GetMeshes()[0].m_Indices.size(), 1, 0, 0,
                         0);
    }
    void Renderer3D::BindDescriptorSets(VkCommandBuffer cmd, Pipeline& pipeline,
                                        VkPipelineBindPoint bindPoint,
                                        u32 firstSet, u32 dSetCount,
                                        u32 dynamicOffsetCount,
                                        u32* dynamicOffsets)
    {
        vkCmdBindDescriptorSets(
            cmd, bindPoint, pipeline.pipelineLayout, firstSet, dSetCount,
            &s_Data.Res.descriptor.Sets[Engine::GetCurrentFrame()],
            dynamicOffsetCount, dynamicOffsets);
    }
    void Renderer3D::BindPipeline(VkCommandBuffer cmd)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          s_Data.api.GraphicsPipeline.pipeline);
    }

    void Renderer3D::Shutdown()
    {
        auto device = Api::GetDevice()->GetDevice();

        s_Data.Res.IndexBuffer->Release();
        delete s_Data.Res.IndexBuffer;
        s_Data.Res.VertexBuffer->Release();
        delete s_Data.Res.VertexBuffer;
        s_Data.Res.DefaultModel.reset();
        for (auto& ub : s_Data.Res.UniformBuffers)
        {
            ub->Release();
            delete ub;
        }
        s_Data.Res.UniformBuffers.clear();
        vkDestroyPipelineLayout(
            device, s_Data.api.GraphicsPipeline.pipelineLayout, nullptr);
        vkDestroyPipeline(device, s_Data.api.GraphicsPipeline.pipeline,
                          nullptr);
    }
    void Renderer3D::UpdateUniformData(UniformBufferObject ubd)
    {
        s_Data.Res.UniformBuffers[Engine::GetCurrentFrame()]->SetData(
            sizeof(ubd), &ubd);
    }

}  // namespace FooGame
