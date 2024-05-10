#include "Renderer2D.h"
#include "../Core/Base.h"
#include "../Core/Engine.h"
#include "../Graphics/Buffer.h"
#include "../Backend/Vertex.h"
#include "../Backend/VulkanCheckResult.h"
#include "../Graphics/Api.h"
#include "../Graphics/Shader.h"
#include "../Core/OrthographicCamera.h"
#include "../Graphics/Image.h"
#include "Core/Core/PerspectiveCamera.h"
#include "vulkan/vulkan_core.h"
namespace FooGame
{

    struct UniformBufferObject
    {
            alignas(16) glm::mat4 Model;
            alignas(16) glm::mat4 View;
            alignas(16) glm::mat4 Projection;
    };
#define VERT_PATH "../../../Shaders/vert.spv"
#define FRAG_PATH "../../../Shaders/frag.spv"
    struct Renderer2DData
    {
            static const u32 MaxQuads    = 20000;
            static const u32 MaxVertices = MaxQuads * 4;
            static const u32 MaxIndices  = MaxQuads * 6;
            struct Resources
            {
                    Buffer* VertexBuffer = nullptr;
                    Buffer* IndexBuffer  = nullptr;
                    List<Buffer*> UniformBuffers;
            };
            Resources resources;
            struct FrameData
            {
                    Vertex* QuadVertexBufferBase = nullptr;
                    Vertex* QuadVertexBufferPtr  = nullptr;
                    glm::vec4 QuadVertexPositions[4];
                    u32 QuadIndexCount = 0;
                    u32 QuadCount      = 0;
                    u32 DrawCall       = 0;
            };
            FrameData frameData;
            Renderer2D::Statistics Stats;
            struct Api
            {
                    GraphicsPipeline Pipeline;
                    VkDescriptorSetLayout DescriptorSetLayout;
                    VkSampler TextureSampler;
                    List<VkDescriptorSet> DescriptorSets;
                    Image image;
            };
            Api api{};
    };
    static Renderer2DData s_Data;
    bool g_IsInitialized = false;
    void Renderer2D::Init()
    {
        assert(g_IsInitialized == false && "Do not init twice!");

        auto* device = Api::GetDevice();
        // create and populate buffers
        {
            u32 offset = 0;
            List<u32> quadIndices(Renderer2DData::MaxIndices);
            for (u32 i = 0; i < Renderer2DData::MaxIndices; i += 6)
            {
                quadIndices[i + 0] = offset + 0;
                quadIndices[i + 1] = offset + 1;
                quadIndices[i + 2] = offset + 2;

                quadIndices[i + 3]  = offset + 2;
                quadIndices[i + 4]  = offset + 3;
                quadIndices[i + 5]  = offset + 0;
                offset             += 4;
            }

            s_Data.resources.IndexBuffer = CreateIndexBuffer(quadIndices);
            quadIndices.clear();

            s_Data.frameData.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f,
                                                       1.0f};
            s_Data.frameData.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
            s_Data.frameData.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
            s_Data.frameData.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
            s_Data.resources.VertexBuffer =
                CreateDynamicBuffer(sizeof(Vertex) * Renderer2DData::MaxIndices,
                                    BufferUsage::VERTEX);
            s_Data.frameData.QuadVertexBufferBase =
                new Vertex[s_Data.MaxVertices];

            s_Data.resources.UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                BufferBuilder uBuffBuilder{};
                uBuffBuilder.SetUsage(BufferUsage::UNIFORM)
                    .SetInitialSize(sizeof(UniformBufferObject))
                    .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
                s_Data.resources.UniformBuffers[i] = CreateDynamicBuffer(
                    sizeof(UniformBufferObject), BufferUsage::UNIFORM);
            }
        }
        //  Create layouts & descriptors for shaders
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
                &s_Data.api.DescriptorSetLayout));
            List<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                                s_Data.api.DescriptorSetLayout);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool     = Api::GetDescriptorPool();
            allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
            allocInfo.pSetLayouts        = layouts.data();

            s_Data.api.DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
            VK_CALL(vkAllocateDescriptorSets(device->GetDevice(), &allocInfo,
                                             s_Data.api.DescriptorSets.data()));

            // texture sampler
            {
                auto props = device->GetPhysicalDeviceProperties();
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.anisotropyEnable = VK_TRUE;
                samplerInfo.maxAnisotropy = props.limits.maxSamplerAnisotropy;
                samplerInfo.borderColor   = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.unnormalizedCoordinates = VK_FALSE;
                samplerInfo.compareEnable           = VK_FALSE;
                samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

                VK_CALL(vkCreateSampler(device->GetDevice(), &samplerInfo,
                                        nullptr, &s_Data.api.TextureSampler));
                LoadTexture(s_Data.api.image, "../../../textures/texture.jpg");
            }
            for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer =
                    *s_Data.resources.UniformBuffers[i]->GetBuffer();
                bufferInfo.offset = 0;
                bufferInfo.range  = sizeof(UniformBufferObject);

                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout =
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = s_Data.api.image.ImageView;
                imageInfo.sampler   = s_Data.api.TextureSampler;

                VkWriteDescriptorSet descriptorWrites[2] = {};
                descriptorWrites[0].sType =
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet     = s_Data.api.DescriptorSets[i];
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType =
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pBufferInfo     = &bufferInfo;

                descriptorWrites[1].sType =
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet     = s_Data.api.DescriptorSets[i];
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pImageInfo      = &imageInfo;

                vkUpdateDescriptorSets(device->GetDevice(),
                                       ARRAY_COUNT(descriptorWrites),
                                       descriptorWrites, 0, nullptr);
            }
        }

        // Create graphics pipeline
        {
            Shader vert{device->GetDevice(), VERT_PATH};
            Shader frag{device->GetDevice(), FRAG_PATH};
            auto vertShaderStageInfo =
                vert.CreateInfo(VK_SHADER_STAGE_VERTEX_BIT);

            auto fragShaderStageInfo =
                frag.CreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT);

            VkPipelineShaderStageCreateInfo shaderStages[] = {
                vertShaderStageInfo, fragShaderStageInfo};

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            auto bindingDescription    = Vertex::GetBindingDescription();
            auto attributeDescriptions = Vertex::GetAttributeDescrp();

            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount =
                static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.pVertexAttributeDescriptions =
                attributeDescriptions.data();

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType =
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType =
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.scissorCount  = 1;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType =
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable        = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth               = 1.0f;
            rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace       = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType =
                VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable  = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineDepthStencilStateCreateInfo depthStencil{};
            depthStencil.sType =
                VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil.depthTestEnable       = VK_TRUE;
            depthStencil.depthWriteEnable      = VK_TRUE;
            depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable     = VK_FALSE;

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType =
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable     = VK_FALSE;
            colorBlending.logicOp           = VK_LOGIC_OP_COPY;
            colorBlending.attachmentCount   = 1;
            colorBlending.pAttachments      = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f;
            colorBlending.blendConstants[1] = 0.0f;
            colorBlending.blendConstants[2] = 0.0f;
            colorBlending.blendConstants[3] = 0.0f;

            VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                              VK_DYNAMIC_STATE_SCISSOR};
            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType =
                VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = ARRAY_COUNT(dynamicStates);
            dynamicState.pDynamicStates    = dynamicStates;

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts    = &s_Data.api.DescriptorSetLayout;

            VK_CALL(vkCreatePipelineLayout(
                device->GetDevice(), &pipelineLayoutInfo, nullptr,
                &s_Data.api.Pipeline.pipelineLayout));
            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType =
                VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount          = 2;
            pipelineInfo.pStages             = shaderStages;
            pipelineInfo.pVertexInputState   = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState      = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState   = &multisampling;
            pipelineInfo.pDepthStencilState  = &depthStencil;
            pipelineInfo.pColorBlendState    = &colorBlending;
            pipelineInfo.pDynamicState       = &dynamicState;
            pipelineInfo.layout     = s_Data.api.Pipeline.pipelineLayout;
            pipelineInfo.renderPass = Api::GetRenderpass();
            pipelineInfo.subpass    = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            VK_CALL(vkCreateGraphicsPipelines(
                device->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                &s_Data.api.Pipeline.pipeline));
        }

        g_IsInitialized = true;
    }

    void Renderer2D::BeginScene(const PerspectiveCamera& camera)
    {
        UniformBufferObject ubd{};

        ubd.Model      = glm::mat4(1.0f);
        ubd.View       = camera.GetView();        /* glm::mat4(1.0f); */
        ubd.Projection = camera.GetProjection();  // glm::mat4(1.0f);
        s_Data.resources.UniformBuffers[Engine::GetCurrentFrame()]->SetData(
            sizeof(ubd), &ubd);
        StartBatch();
    }
    void Renderer2D::BeginScene(const OrthographicCamera& camera)
    {
        UniformBufferObject ubd{};

        ubd.Model      = glm::mat4(1.0f);
        ubd.View       = camera.GetView();        // glm::mat4(1.0f);
        ubd.Projection = camera.GetProjection();  // glm::mat4(1.0f);
        s_Data.resources.UniformBuffers[Engine::GetCurrentFrame()]->SetData(
            sizeof(ubd), &ubd);
        StartBatch();
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
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), position) *
            glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        DrawQuad(transform, color);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform,
                              const glm::vec4& color)
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
            s_Data.frameData.QuadVertexBufferPtr->Color =
                glm::vec3{1.0f, 1.0f, 1.0f};
            s_Data.frameData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.frameData.QuadVertexBufferPtr++;
        }
        s_Data.frameData.QuadIndexCount += 6;
        s_Data.frameData.QuadCount++;
    }
    void Renderer2D::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer2D::StartBatch()
    {
        s_Data.frameData.QuadIndexCount = 0;
        s_Data.frameData.QuadVertexBufferPtr =
            s_Data.frameData.QuadVertexBufferBase;
    }
    void Renderer2D::BeginRecording(VkCommandBuffer cb)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VK_CALL(vkBeginCommandBuffer(cb, &beginInfo));
    }
    void Renderer2D::BeginDrawing()
    {
        s_Data.frameData.QuadIndexCount = 0;
        s_Data.frameData.QuadVertexBufferPtr =
            s_Data.frameData.QuadVertexBufferBase;
    }
    void Renderer2D::EndDrawing()
    {
        auto currentFrame = Engine::GetCurrentFrame();
        auto cmd          = Engine::GetCurrentCommandbuffer();
    }

    void Renderer2D::Flush()
    {
        auto currentFrame = Engine::GetCurrentFrame();
        auto cmd          = Engine::GetCurrentCommandbuffer();
        auto extent       = Engine::GetSwapchainExtent();
        BindPipeline(cmd);

        Api::SetViewportAndScissors(cmd, extent.width, extent.height);
        // bind vertexbuffers
        VkBuffer vertexBuffers[] = {
            *s_Data.resources.VertexBuffer->GetBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        // bind index buffers
        vkCmdBindIndexBuffer(cmd, *s_Data.resources.IndexBuffer->GetBuffer(), 0,
                             VK_INDEX_TYPE_UINT32);
        // bind descriptorsets

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                s_Data.api.Pipeline.pipelineLayout, 0, 1,
                                &s_Data.api.DescriptorSets[currentFrame], 0,
                                nullptr);
        if (s_Data.frameData.QuadIndexCount)
        {
            u32 dataSize =
                (u32)((uint8_t*)s_Data.frameData.QuadVertexBufferPtr -
                      (uint8_t*)s_Data.frameData.QuadVertexBufferBase);
            s_Data.resources.VertexBuffer->SetData(
                dataSize, s_Data.frameData.QuadVertexBufferBase);
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
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          s_Data.api.Pipeline.pipeline);
    }

    Renderer2D::Statistics Renderer2D::GetStats()
    {
        return s_Data.Stats;
    }
    void Renderer2D::Shutdown()
    {
        auto device = Api::GetDevice()->GetDevice();
        vkDestroyDescriptorSetLayout(device, s_Data.api.DescriptorSetLayout,
                                     nullptr);
        s_Data.resources.IndexBuffer->Release();
        delete s_Data.resources.IndexBuffer;
        s_Data.resources.VertexBuffer->Release();
        delete s_Data.resources.VertexBuffer;
        for (auto& ub : s_Data.resources.UniformBuffers)
        {
            ub->Release();
        }
        s_Data.resources.UniformBuffers.clear();
        vkDestroyPipelineLayout(device, s_Data.api.Pipeline.pipelineLayout,
                                nullptr);
        vkDestroyPipeline(device, s_Data.api.Pipeline.pipeline, nullptr);
        vkDestroySampler(device, s_Data.api.TextureSampler, nullptr);
        DestroyImage(s_Data.api.image);
        delete[] s_Data.frameData.QuadVertexBufferBase;
        // TODO: clear created resources
    }
}  // namespace FooGame
