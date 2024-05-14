#include "Renderer3D.h"
#include "Api.h"
#include "Buffer.h"
#include "../Backend/Vertex.h"
#include "../Core/Base.h"
#include "../Core/Engine.h"
#include "../Graphics/Model.h"
#include "../Graphics/Pipeline.h"
#include "../Graphics/Texture2D.h"
#include "Core/Core/PerspectiveCamera.h"
#include "Shader.h"
#include <cassert>
#include "../Core/Window.h"
#include "vulkan/vulkan_core.h"
namespace FooGame
{
#if 0
#define VERT_SHADER "../../../Shaders/vert.spv"
#define FRAG_SHADER "../../../Shaders/frag.spv"
#define MODEL_PATH  "../../../Assets/Model/viking_room.obj"
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
                    Buffer* VertexBuffer       = nullptr;
                    Buffer* IndexBuffer        = nullptr;
                    Shared<Model> DefaultModel = nullptr;
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
            info.DescriptorSetLayout = *Engine::GetDescriptorSetLayout();
            info.pushConstantSize    = sizeof(MeshPushConstants);
            info.pushConstantCount   = 1;

            s_Data.api.GraphicsPipeline = CreateGraphicsPipeline(info);
            s_Data.Res.DefaultModel     = Model::LoadModel(MODEL_PATH);
            SubmitModel(s_Data.Res.DefaultModel);
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
        Engine::UpdateUniformData(ubd);
    }

    void Renderer3D::EndScene()
    {
        Flush();
    }
    void Renderer3D::Flush()
    {
    }
    void Renderer3D::DrawModel()
    {
        DrawModel(*s_Data.Res.DefaultModel);
    }
    void Renderer3D::DrawModel(const Model& model)
    {
        auto currentFrame = Engine::GetCurrentFrame();
        auto cmd          = Engine::GetCurrentCommandbuffer();
        auto extent       = Engine::GetSwapchainExtent();

        BindPipeline(cmd);

        Api::SetViewportAndScissors(cmd, extent.width, extent.height);
        // bind vertexbuffers
        VkBuffer vertexBuffers[] = {*s_Data.Res.VertexBuffer->GetBuffer()};
        VkDeviceSize offsets[]   = {0};
        MeshPushConstants push{};
        push.renderMatrix = glm::rotate(
            glm::mat4(1.0f),
            glm::radians(45.0f) * (float)WindowsWindow::Get().GetTime(),
            glm::vec3{0.5f, 0.3f, .2f});
        vkCmdPushConstants(cmd, s_Data.api.GraphicsPipeline.pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(MeshPushConstants), &push);
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        // bind index buffers
        vkCmdBindIndexBuffer(cmd, *s_Data.Res.IndexBuffer->GetBuffer(), 0,
                             VK_INDEX_TYPE_UINT32);
        // bind descriptorsets
        Engine::BindDescriptorSets(cmd, s_Data.api.GraphicsPipeline);
        vkCmdDrawIndexed(cmd, model.GetMeshes()[0].m_Indices.size(), 1, 0, 0,
                         0);
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
        delete s_Data.Res.VertexBuffer;
        s_Data.Res.IndexBuffer->Release();
        delete s_Data.Res.IndexBuffer;
        s_Data.Res.DefaultModel.reset();
        vkDestroyPipelineLayout(
            device, s_Data.api.GraphicsPipeline.pipelineLayout, nullptr);
        vkDestroyPipeline(device, s_Data.api.GraphicsPipeline.pipeline,
                          nullptr);
    }

}  // namespace FooGame
