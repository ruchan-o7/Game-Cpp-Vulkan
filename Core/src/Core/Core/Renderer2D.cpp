#include "Renderer2D.h"
#include <vulkan/vulkan_core.h>
#include <xerrc.h>
#include "../Backend/VBackend.h"
#include "../Core/Base.h"
#include "../Backend/Buffer.h"
#include "../Backend/Vertex.h"
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

            Shared<Buffer> QuadVertexBuffer;
            QuadVertex* QuadVertexBufferBase;
            QuadVertex* QuadVertexBufferPtr = nullptr;
            glm::vec4 QuadVertexPositions[4];

            Shared<Buffer> QuadIndexBuffer;
            u32 QuadIndexCount = 0;

            Renderer2D::Statistics Stats;

            struct CameraData
            {
                    glm::mat4 ViewProjection;
            };
            CameraData CameraBuffer;
    };
    static Renderer2DData s_Data;
    static Context* s_Context = nullptr;
    void Renderer2D::Init()
    {
        s_Context = new Context;
        s_Context->Init();
        VkPhysicalDeviceMemoryProperties props;
        auto device = s_Context->GetDevice().physical_device;
        vkGetPhysicalDeviceMemoryProperties(device, &props);
        auto usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        auto flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        s_Data.QuadVertexBuffer = CreateShared<Buffer>(
            s_Data.MaxVertices * sizeof(QuadVertex), props, usage, flags);
        s_Context->BindVertexBuffer(s_Data.QuadVertexBuffer->GetBuffer());

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
        usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        s_Data.QuadIndexBuffer = CreateShared<Buffer>(
            sizeof(u32) * s_Data.MaxIndices, props, usage, flags);
        s_Context->BindIndexBuffer(s_Data.QuadIndexBuffer->GetBuffer());
        delete[] quadIndices;

        s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
    }

    // void Renderer2D::DrawFrame()
    // {
    //     s_Context->DrawFrame();
    // }
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
    void Renderer2D::BeginDraw()
    {
        ResetStats();
        StartBatch();
        s_Context->BeginDraw();
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
        if (s_Data.QuadIndexCount)
        {
            uint32_t dataSize =
                (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr -
                           (uint8_t*)s_Data.QuadVertexBufferBase);
            s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase,
                                             dataSize);
            // s_Context->DrawIndexed(s_Data.QuadIndexCount,
            //                        s_Data.Stats.QuadCount);
            s_Context->DrawNotIndexed(s_Data.Stats.GetTotalVertexCount(), 1, 0);
            s_Data.Stats.DrawCalls++;
        }
        s_Context->EndDraw();
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
