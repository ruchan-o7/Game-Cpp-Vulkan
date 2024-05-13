#include "Renderer3D.h"
#include "Api.h"
#include "Buffer.h"
#include "../Backend/Vertex.h"
#include "../Core/Base.h"
#include "../Core/Engine.h"
#include "../Graphics/Model.h"
#include "../Graphics/Pipeline.h"
#include "../Graphics/Texture2D.h"
#include "Shader.h"
#include <cassert>
namespace FooGame
{

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
            Shader vert{"../../../Shaders/vert.spv", ShaderStage::VERTEX};
            Shader frag{"../../../Shaders/frag.spv", ShaderStage::FRAGMENT};
            info.Shaders = List<Shader*>{&vert, &frag};
            info.VertexAttributeDescriptons =
                Vertex::GetAttributeDescriptionList();
            info.VertexBindings      = {Vertex::GetBindingDescription()};
            info.LineWidth           = 2.0f;
            info.CullMode            = CullMode::BACK;
            info.MultiSampling       = MultiSampling::LEVEL_1;
            info.DescriptorSetLayout = *Engine::GetDescriptorSetLayout();

            s_Data.api.GraphicsPipeline = CreateGraphicsPipeline(info);
            s_Data.Res.DefaultModel =
                Model::LoadModel("../../../Assets/Model/viking_room.obj");
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
}  // namespace FooGame
