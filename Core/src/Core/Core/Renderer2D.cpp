#include "Renderer2D.h"
#include <vulkan/vulkan_core.h>
#include <xerrc.h>
#include "../Backend/VBackend.h"
#include "../Core/Base.h"
#include "../Backend/Buffer.h"
#include "../Backend/Vertex.h"
#include "GLFW/glfw3.h"
#include <cstring>
#include <glm/ext/matrix_transform.hpp>
namespace FooGame
{
    struct Renderer2DData
    {
            static const u32 MaxQuads        = 20000;
            static const u32 MaxVertices     = MaxQuads * 4;
            static const u32 MaxIndices      = MaxQuads * 6;
            static const u32 MaxTextureSlots = 32;

            Buffer QuadVertexBuffer;
            QuadVertex* QuadVertexBufferBase;
            QuadVertex* QuadVertexBufferPtr = nullptr;
            glm::vec4 QuadVertexPositions[4];

            Buffer QuadIndexBuffer;
            u32 QuadIndexCount = 0;

            Renderer2D::Statistics Stats;

            struct CameraData
            {
                    glm::mat4 ViewProjection;
            };
            CameraData CameraBuffer;
            Buffer CameraUniformBuffer;
    };
    static Renderer2DData s_Data;
    static API* s_Context = nullptr;
    void Renderer2D::Init()
    {
        s_Context = new API;
        s_Context->Init();
        VkPhysicalDeviceMemoryProperties props;
        auto device = s_Context->GetPhysicalDevice();
        vkGetPhysicalDeviceMemoryProperties(device, &props);

        auto quadVertexBufferSize = s_Data.MaxVertices * sizeof(QuadVertex);

        CreateBuffer(s_Data.QuadVertexBuffer, s_Context->GetDevice(), props,
                     sizeof(Renderer2DData::CameraData),
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        // CreateBuffer(s_Data.QuadVertexBuffer, s_Context->GetDevice(), props,
        //              quadVertexBufferSize,
        //              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        //              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        s_Context->SetVertexBuffer(s_Data.QuadVertexBuffer.buf);

        s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

        u32* quadIndices = new u32[s_Data.MaxIndices];
        u32 offset       = 0;
        for (u32 i = 0; i < s_Data.MaxIndices; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3]  = offset + 3;
            quadIndices[i + 4]  = offset + 4;
            quadIndices[i + 5]  = offset + 5;
            offset             += 4;
        }
        CreateBuffer(s_Data.QuadIndexBuffer, s_Context->GetDevice(), props,
                     sizeof(u32) * s_Data.MaxIndices,
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        // CreateIndexBuffer(s_Data.QuadIndexBuffer, s_Context->GetDevice(),
        // props,
        //                   sizeof(u32) * s_Data.MaxIndices,
        //                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        s_Context->BindIndexBuffer(s_Data.QuadIndexBuffer.buf);
        UpdateBufferData(s_Data.QuadIndexBuffer, quadIndices,
                         sizeof(u32) * s_Data.MaxIndices);
        delete[] quadIndices;

        s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
        CreateBuffer(s_Data.CameraUniformBuffer, s_Context->GetDevice(), props,
                     sizeof(Renderer2DData::CameraData),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform,
                              const glm::vec4& color)
    {
        constexpr size_t quadVertexCount = 4;
        const float textureIndex         = 0.0f;
        // constexpr glm::vec2 textureCoords[] = {
        //     {0.0f, 0.0f},
        //     {1.0f, 0.0f},
        //     {1.0f, 1.0f},
        //     {0.0f, 1.0f}
        // };
        // const float tilingFactor = 1.0f;
        // TODO:check for next batch
        for (size_t i = 0; i < quadVertexCount; ++i)
        {
            s_Data.QuadVertexBufferPtr->Position =
                transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr++;
        }
        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }
    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                              const glm::vec4& color)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }
    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size,
                              const glm::vec4& color)
    {
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), position) *
            glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        DrawQuad(transform, color);
    }

    void Renderer2D::Resize()
    {
        s_Context->ResizeSwapchain();
    }

    void Renderer2D::SetClearColor(glm::vec3 color)
    {
        s_Context->SetClearColor({color.x, color.y, color.z});
    }
    double deltaTime     = 0;
    double lastFrameTime = 0;
    void Renderer2D::BeginScene(const Camera& camera)
    {
        double currentTime = glfwGetTime();
        deltaTime          = currentTime - lastFrameTime;
        lastFrameTime      = currentTime;

        s_Data.CameraBuffer.ViewProjection = camera.GetProjection();
        UpdateBufferData(s_Data.CameraUniformBuffer,
                         &s_Data.CameraBuffer.ViewProjection,
                         sizeof(Renderer2DData::CameraBuffer));
        StartBatch();
    }

    VkDevice Renderer2D::GetDevice()
    {
        return s_Context->GetDevice();
    }
    void Renderer2D::EndDraw()
    {
        Flush();
    }

    void Renderer2D::ResetStats()
    {
        memset(&s_Data.Stats, 0, sizeof(Statistics));
    }
    void Renderer2D::Flush()
    {
        s_Context->WaitForNewImage();
        s_Context->StartRecording();
        if (s_Data.QuadIndexCount)
        {
            uint32_t dataSize =
                (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr -
                           (uint8_t*)s_Data.QuadVertexBufferBase);
            UpdateBufferData(s_Data.QuadVertexBuffer,
                             s_Data.QuadVertexBufferBase, dataSize);
            // s_Context->Draw(s_Data.Stats.GetTotalVertexCount(),
            //                 s_Data.Stats.QuadCount, 0);
            s_Context->DrawIndexed(s_Data.Stats.QuadCount, 1, 0);
            s_Data.Stats.DrawCalls++;
        }
        s_Context->StopRecording();
        s_Context->Submit();
    }
    void Renderer2D::StartBatch()
    {
        s_Data.QuadIndexCount      = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
    }
    void Renderer2D::Shutdown()
    {
        // TODO:
        delete[] s_Data.QuadVertexBufferBase;
    }

}  // namespace FooGame
