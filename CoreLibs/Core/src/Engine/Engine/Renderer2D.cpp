#include "Renderer2D.h"
#include "../Geometry/QuadVertex.h"
#include "Types/GraphicTypes.h"
#include "Types/DescriptorData.h"
#include "Backend.h"
#include <cstring>
#include "VulkanCheckResult.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
namespace FooGame
{

#define VERT_PATH "Assets/Shaders/QuadShaderVert.spv"
#define FRAG_PATH "Assets/Shaders/QuadShaderFrag.spv"
    struct Renderer2DData
    {
            static const uint32_t MaxQuads    = 20000;
            static const uint32_t MaxVertices = MaxQuads * 4;
            static const uint32_t MaxIndices  = MaxQuads * 6;
            struct Resources
            {
                    VkDescriptorSet set[3];
                    DescriptorData descriptor;
            };
            Resources resources;
            struct FrameData
            {
                    QuadVertex* QuadVertexBufferBase = nullptr;
                    QuadVertex* QuadVertexBufferPtr  = nullptr;
                    glm::vec4 QuadVertexPositions[4];
                    std::shared_ptr<Texture2D> DefaultTexture;
                    uint32_t QuadIndexCount = 0;
                    uint32_t QuadCount      = 0;
                    uint32_t DrawCall       = 0;
            };
            FrameData frameData;
            Renderer2D::Statistics Stats;
            struct Api
            {
                    // Pipeline pipeline;
                    VkSampler TextureSampler;
            };
            Api api{};
    };
    static Renderer2DData s_Data;
    bool g_IsInitialized = false;
    void Renderer2D::Init()
    {
        assert(g_IsInitialized == false && "Do not init twice!");

        auto* device = Backend::GetRenderDevice()->GetVkDevice();  // Api::GetDevice();
        // create and populate buffers
        {
            uint32_t offset = 0;
            std::vector<uint32_t> quadIndices(Renderer2DData::MaxIndices);
            for (uint32_t i = 0; i < Renderer2DData::MaxIndices; i += 6)
            {
                quadIndices[i + 0] = offset + 0;
                quadIndices[i + 1] = offset + 1;
                quadIndices[i + 2] = offset + 2;

                quadIndices[i + 3]  = offset + 2;
                quadIndices[i + 4]  = offset + 3;
                quadIndices[i + 5]  = offset + 0;
                offset             += 4;
            }

            // s_Data.resources.IndexBuffer = CreateIndexBuffer(quadIndices);
            quadIndices.clear();
            s_Data.frameData.QuadVertexPositions[0] = {-1.0f, 1.0f, 0.0f, 1.0f};
            s_Data.frameData.QuadVertexPositions[1] = {-1.0f, -1.0f, 0.0f, 1.0f};
            s_Data.frameData.QuadVertexPositions[2] = {1.0f, -1.0f, 0.0f, 1.0f};
            s_Data.frameData.QuadVertexPositions[3] = {1.0f, 1.0f, 0.0f, 1.0f};
            // s_Data.resources.VertexBuffer           = CreateDynamicBuffer(
            //     sizeof(QuadVertex) * Renderer2DData::MaxIndices, BufferUsage::VERTEX);
            // s_Data.frameData.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];
        }
        {
            // s_Data.resources.UniformBuffers.resize(2);

            for (uint32_t i = 0; i < 2; i++)
            {
                // BufferBuilder uBuffBuilder{};
                // uBuffBuilder.SetUsage(BufferUsage::UNIFORM)
                //     .SetInitialSize(sizeof(UniformBufferObject))
                //     .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                //                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                // s_Data.resources.UniformBuffers[i] =
                //     CreateDynamicBuffer(sizeof(UniformBufferObject), BufferUsage::UNIFORM);
            }
        }
        {
            VkDescriptorPoolSize poolSizes[1] = {};
            poolSizes[0].type                 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount      = 2;

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = ARRAY_COUNT(poolSizes);
            poolInfo.pPoolSizes    = poolSizes;
            poolInfo.maxSets       = 2;
            VK_CALL(vkCreateDescriptorPool(device, &poolInfo, nullptr,
                                           &s_Data.resources.descriptor.pool));
        }
        {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding                 = 0;
            uboLayoutBinding.descriptorCount         = 1;
            uboLayoutBinding.descriptorType          = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers      = nullptr;
            uboLayoutBinding.stageFlags              = VK_SHADER_STAGE_VERTEX_BIT;
            VkDescriptorSetLayoutBinding bindings[1] = {uboLayoutBinding};
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = ARRAY_COUNT(bindings);
            layoutInfo.pBindings    = bindings;

            VK_CALL(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                                &s_Data.resources.descriptor.SetLayout));
            std::vector<VkDescriptorSetLayout> layouts(2, s_Data.resources.descriptor.SetLayout);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool     = s_Data.resources.descriptor.pool;
            allocInfo.descriptorSetCount = 2;
            allocInfo.pSetLayouts        = layouts.data();

            VK_CALL(vkAllocateDescriptorSets(device, &allocInfo, s_Data.resources.set));
        }
        {
            for (uint32_t i = 0; i < 2; i++)
            {
                VkDescriptorBufferInfo bufferInfo{};
                // bufferInfo.buffer = *s_Data.resources.UniformBuffers[i]->GetBuffer();
                bufferInfo.offset = 0;
                bufferInfo.range  = sizeof(UniformBufferObject);

                VkWriteDescriptorSet descriptorWrites[1] = {};
                descriptorWrites[0].sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet               = s_Data.resources.set[i];
                descriptorWrites[0].dstBinding           = 0;
                descriptorWrites[0].dstArrayElement      = 0;
                descriptorWrites[0].descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[0].descriptorCount      = 1;
                descriptorWrites[0].pBufferInfo          = &bufferInfo;

                vkUpdateDescriptorSets(device, ARRAY_COUNT(descriptorWrites), descriptorWrites, 0,
                                       nullptr);
            }
        }

        // Create graphics pipeline
        {
            // Shader vert{VERT_PATH, ShaderStage::VERTEX};
            // Shader frag{FRAG_PATH, ShaderStage::FRAGMENT};
            // PipelineInfo info{};
            // info.Shaders = {&vert, &frag};

            // info.VertexAttributeDescriptons =
            //     QuadVertex::GetAttributeDescriptionList();
            // info.VertexBindings      = {QuadVertex::GetBindingDescription()};
            // info.LineWidth           = 2.0f;
            // info.cullMode            = CullMode::BACK;
            // info.multiSampling       = MultiSampling::LEVEL_1;
            // info.DescriptorSetLayout = s_Data.resources.descriptor.SetLayout;
            // s_Data.api.pipeline      = CreateGraphicsPipeline(info);
        }

        g_IsInitialized = true;
    }  // namespace FooGame

    void Renderer2D::BeginScene(const PerspectiveCamera& camera)
    {
        UniformBufferObject ubd{};

        ubd.View       = camera.GetView();
        ubd.Projection = camera.GetProjection();
        UpdateUniformData(ubd);
        StartBatch();
    }
    void Renderer2D::BeginScene(const OrthographicCamera& camera)
    {
        UniformBufferObject ubd{};
        ubd.View       = camera.GetView();
        ubd.Projection = camera.GetProjection();
        UpdateUniformData(ubd);
        StartBatch();
    }
    void Renderer2D::UpdateUniformData(UniformBufferObject ubd)
    {
        // s_Data.resources.UniformBuffers[Backend::GetCurrentFrame()]->SetData(sizeof(ubd), &ubd);
    }

    void Renderer2D::EndScene()
    {
        Flush();
    }
    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                              const glm::vec4& color)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size,
                              const glm::vec4& color)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                              glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        DrawQuad(transform, color);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
    {
        constexpr size_t quadVertexCount    = 4;
        constexpr glm::vec2 textureCoords[] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };
        if (s_Data.frameData.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            NextBatch();
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.frameData.QuadVertexBufferPtr->Position =
                transform * s_Data.frameData.QuadVertexPositions[i];
            s_Data.frameData.QuadVertexBufferPtr->Color        = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
            s_Data.frameData.QuadVertexBufferPtr->TexCoord     = textureCoords[i];
            s_Data.frameData.QuadVertexBufferPtr->TilingFactor = 0.5f;
            s_Data.frameData.QuadVertexBufferPtr++;
        }
        s_Data.frameData.QuadIndexCount += 6;
        s_Data.frameData.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                              const std::shared_ptr<Texture2D>& texture, float tilingFactor,
                              const glm::vec4& tintColor)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, texture, tilingFactor, tintColor);
    }
    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size,
                              const std::shared_ptr<Texture2D>& texture, float tilingFactor,
                              const glm::vec4& tintColor)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                              glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        DrawQuad(transform, texture, tilingFactor, tintColor);
    }
    void Renderer2D::DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture,
                              float tilingFactor, const glm::vec4& tintColor)
    {
        constexpr size_t quadVertexCount    = 4;
        constexpr glm::vec2 textureCoords[] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };

        if (s_Data.frameData.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            NextBatch();
        }

        float textureIndex = 0.0f;
        // for (uint32_t i = 1; i < s_Data.frameData.TextureSlotIndex; i++)
        // {
        //     if (s_Data.frameData.TextureSlots[i]->Image == texture->Image)
        //     {
        //         textureIndex = (float)i;
        //         break;
        //     }
        // }
        //
        // if (textureIndex == 0.0f)
        // {
        //     if (s_Data.frameData.TextureSlotIndex >=
        //         TEXTURE_SAMPLER_ARRAY_COUNT)
        //     {
        //         NextBatch();
        //     }
        //
        //     textureIndex = (float)s_Data.frameData.TextureSlotIndex;
        //     s_Data.frameData.TextureSlots[s_Data.frameData.TextureSlotIndex]
        //     =
        //         texture;
        //     s_Data.frameData.TextureSlotIndex++;
        // }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.frameData.QuadVertexBufferPtr->Position =
                transform * s_Data.frameData.QuadVertexPositions[i];
            s_Data.frameData.QuadVertexBufferPtr->Color        = tintColor;
            s_Data.frameData.QuadVertexBufferPtr->TexCoord     = textureCoords[i];
            s_Data.frameData.QuadVertexBufferPtr->TexIndex     = textureIndex;
            s_Data.frameData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            // s_Data.frameData.QuadVertexBufferPtr->EntityID     = entityID;
            s_Data.frameData.QuadVertexBufferPtr++;
        }

        s_Data.frameData.QuadIndexCount += 6;

        s_Data.Stats.QuadCount++;
    }
    void Renderer2D::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer2D::StartBatch()
    {
        s_Data.frameData.QuadIndexCount      = 0;
        s_Data.frameData.QuadVertexBufferPtr = s_Data.frameData.QuadVertexBufferBase;
    }
    void Renderer2D::BeginRecording(VkCommandBuffer cb)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VK_CALL(vkBeginCommandBuffer(cb, &beginInfo));
    }
    void Renderer2D::BeginDraw()
    {
        s_Data.frameData.QuadIndexCount      = 0;
        s_Data.frameData.QuadVertexBufferPtr = s_Data.frameData.QuadVertexBufferBase;
    }
    void Renderer2D::EndDraw()
    {
        auto currentFrame = Backend::GetCurrentFrame();
        auto cmd          = Backend::GetCurrentCommandbuffer();
    }

    void Renderer2D::Flush()
    {
        auto currentFrame = Backend::GetCurrentFrame();
        auto cmd          = Backend::GetCurrentCommandbuffer();
        auto extent       = Backend::GetSwapchainExtent();
        BindPipeline(cmd);

        // Api::SetViewportAndScissors(cmd, extent.width, extent.height);
        // bind vertexbuffers
        // VkBuffer vertexBuffers[] = {*s_Data.resources.VertexBuffer->GetBuffer()};
        VkDeviceSize offsets[] = {0};
        // vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        // bind index buffers
        // vkCmdBindIndexBuffer(cmd, *s_Data.resources.IndexBuffer->GetBuffer(), 0,
        //                      VK_INDEX_TYPE_UINT32);
        // bind descriptorsets
        // BindDescriptorSets(cmd, s_Data.api.pipeline);
        if (s_Data.frameData.QuadIndexCount)
        {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.frameData.QuadVertexBufferPtr -
                                           (uint8_t*)s_Data.frameData.QuadVertexBufferBase);
            // s_Data.resources.VertexBuffer->SetData(dataSize,
            // s_Data.frameData.QuadVertexBufferBase);
            vkCmdDrawIndexed(cmd, s_Data.frameData.QuadIndexCount, 2, 0, 0, 0);
            s_Data.frameData.DrawCall++;
        }
    }

    void Renderer2D::ResetStats()
    {
        memset(&s_Data.Stats, 0, sizeof(Statistics));
    }
    void Renderer2D::BindPipeline(VkCommandBuffer cmd)
    {
        // vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, s_Data.api.pipeline.pipeline);
    }

    Renderer2D::Statistics Renderer2D::GetStats()
    {
        return s_Data.Stats;
    }

    void Renderer2D::Shutdown()
    {
        // auto device = Api::GetDevice()->GetDevice();
        // s_Data.resources.IndexBuffer->Release();
        // delete s_Data.resources.IndexBuffer;
        // s_Data.resources.VertexBuffer->Release();
        // delete s_Data.resources.VertexBuffer;
        // for (auto& ub : s_Data.resources.UniformBuffers)
        // {
        //     ub->Release();
        //     delete ub;
        // }
        // s_Data.resources.UniformBuffers.clear();
        // vkDestroyPipelineLayout(device, s_Data.api.pipeline.pipelineLayout, nullptr);
        // vkDestroyPipeline(device, s_Data.api.pipeline.pipeline, nullptr);
        // vkDestroySampler(device, s_Data.api.TextureSampler, nullptr);
        // // DestroyImage(s_Data.frameData.DefaultTexture.get());
        // delete[] s_Data.frameData.QuadVertexBufferBase;
    }
}  // namespace FooGame
