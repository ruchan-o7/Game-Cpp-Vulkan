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
        // {
        //     VkDescriptorSetLayoutBinding uboLayoutBinding{};
        //     uboLayoutBinding.binding         = 0;
        //     uboLayoutBinding.descriptorCount = 1;
        //     uboLayoutBinding.descriptorType =
        //     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        //     uboLayoutBinding.pImmutableSamplers = nullptr;
        //     uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        //
        //     VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        //     samplerLayoutBinding.binding         = 1;
        //     samplerLayoutBinding.descriptorCount = 1;
        //     samplerLayoutBinding.descriptorType =
        //         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        //     samplerLayoutBinding.pImmutableSamplers = nullptr;
        //     samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        //
        //     VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding,
        //                                                 samplerLayoutBinding};
        //     VkDescriptorSetLayoutCreateInfo layoutInfo{};
        //     layoutInfo.sType =
        //         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        //     layoutInfo.bindingCount = ARRAY_COUNT(bindings);
        //     layoutInfo.pBindings    = bindings;
        //
        //     VK_CALL(vkCreateDescriptorSetLayout(
        //         device->GetDevice(), &layoutInfo, nullptr,
        //         &s_Data.api.DescriptorSetLayout));
        //     List<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
        //                                         s_Data.api.DescriptorSetLayout);
        //     VkDescriptorSetAllocateInfo allocInfo{};
        //     allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        //     allocInfo.descriptorPool     = Api::GetDescriptorPool();
        //     allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
        //     allocInfo.pSetLayouts        = layouts.data();
        //
        //     s_Data.api.DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        //     VK_CALL(vkAllocateDescriptorSets(device->GetDevice(), &allocInfo,
        //                                      s_Data.api.DescriptorSets.data()));
        //
        //     // texture sampler
        //     {
        //         auto props = device->GetPhysicalDeviceProperties();
        //         VkSamplerCreateInfo samplerInfo{};
        //         samplerInfo.sType     =
        //         VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO; samplerInfo.magFilter
        //         = VK_FILTER_LINEAR; samplerInfo.minFilter = VK_FILTER_LINEAR;
        //         samplerInfo.addressModeU     =
        //         VK_SAMPLER_ADDRESS_MODE_REPEAT; samplerInfo.addressModeV =
        //         VK_SAMPLER_ADDRESS_MODE_REPEAT; samplerInfo.addressModeW =
        //         VK_SAMPLER_ADDRESS_MODE_REPEAT; samplerInfo.anisotropyEnable
        //         = VK_TRUE; samplerInfo.maxAnisotropy =
        //         props.limits.maxSamplerAnisotropy; samplerInfo.borderColor =
        //         VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        //         samplerInfo.unnormalizedCoordinates = VK_FALSE;
        //         samplerInfo.compareEnable           = VK_FALSE;
        //         samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        //         samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        //
        //         VK_CALL(vkCreateSampler(device->GetDevice(), &samplerInfo,
        //                                 nullptr,
        //                                 &s_Data.api.TextureSampler));
        //
        //         s_Data.api.DefaultTexture =
        //             LoadTexture("../../../textures/texture.jpg");
        //     }
        //
        //     for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        //     {
        //         VkDescriptorBufferInfo bufferInfo{};
        //         bufferInfo.buffer =
        //         *s_Data.Res.UniformBuffers[i]->GetBuffer(); bufferInfo.offset
        //         = 0; bufferInfo.range  = sizeof(UniformBufferObject);
        //
        //         VkDescriptorImageInfo imageInfo{};
        //
        //         imageInfo.imageLayout =
        //             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        //         imageInfo.imageView = s_Data.api.DefaultTexture->ImageView;
        //         imageInfo.sampler   = s_Data.api.TextureSampler;
        //
        //         VkWriteDescriptorSet descriptorWrites[2] = {};
        //         descriptorWrites[0].sType =
        //             VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //         descriptorWrites[0].dstSet     =
        //         s_Data.api.DescriptorSets[i]; descriptorWrites[0].dstBinding
        //         = 0; descriptorWrites[0].dstArrayElement = 0;
        //         descriptorWrites[0].descriptorType =
        //             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        //         descriptorWrites[0].descriptorCount = 1;
        //         descriptorWrites[0].pBufferInfo     = &bufferInfo;
        //
        //         descriptorWrites[1].sType =
        //             VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //         descriptorWrites[1].dstSet     =
        //         s_Data.api.DescriptorSets[i]; descriptorWrites[1].dstBinding
        //         = 1; descriptorWrites[1].dstArrayElement = 0;
        //         descriptorWrites[1].descriptorType =
        //             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        //         descriptorWrites[1].descriptorCount = 1;
        //         // TEXTURE_SAMPLER_ARRAY_COUNT;
        //         descriptorWrites[1].pImageInfo = &imageInfo;
        //
        //         vkUpdateDescriptorSets(device->GetDevice(),
        //                                ARRAY_COUNT(descriptorWrites),
        //                                descriptorWrites, 0, nullptr);
        //     }
        // }
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
    // GraphicsPipeline CreateGraphicsPipeline(Shader& vertexShader,
    //                                         Shader& fragmentShader)
    // {
    //     auto device = Api::GetDevice()->GetDevice();
    //     GraphicsPipeline pipeline{};
    //
    //     auto vertShaderStageInfo = vertexShader.CreateInfo();
    //
    //     auto fragShaderStageInfo = fragmentShader.CreateInfo();
    //
    //     VkPipelineShaderStageCreateInfo shaderStages[] =
    //     {vertShaderStageInfo,
    //                                                       fragShaderStageInfo};
    //
    //     VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    //     vertexInputInfo.sType =
    //         VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    //     auto bindingDescription    = Vertex::GetBindingDescription();
    //     auto attributeDescriptions = Vertex::GetAttributeDescrp();
    //
    //     vertexInputInfo.vertexBindingDescriptionCount = 1;
    //     vertexInputInfo.vertexAttributeDescriptionCount =
    //         static_cast<uint32_t>(attributeDescriptions.size());
    //     vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    //     vertexInputInfo.pVertexAttributeDescriptions =
    //         attributeDescriptions.data();
    //
    //     VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    //     inputAssembly.sType =
    //         VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    //     inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    //     inputAssembly.primitiveRestartEnable = VK_FALSE;
    //
    //     VkPipelineViewportStateCreateInfo viewportState{};
    //     viewportState.sType =
    //         VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    //     viewportState.viewportCount = 1;
    //     viewportState.scissorCount  = 1;
    //
    //     VkPipelineRasterizationStateCreateInfo rasterizer{};
    //     rasterizer.sType =
    //         VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    //     rasterizer.depthClampEnable        = VK_FALSE;
    //     rasterizer.rasterizerDiscardEnable = VK_FALSE;
    //     rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    //     rasterizer.lineWidth               = 1.0f;
    //     rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    //     rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    //     rasterizer.depthBiasEnable         = VK_FALSE;
    //
    //     VkPipelineMultisampleStateCreateInfo multisampling{};
    //     multisampling.sType =
    //         VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    //     multisampling.sampleShadingEnable  = VK_FALSE;
    //     multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    //
    //     VkPipelineDepthStencilStateCreateInfo depthStencil{};
    //     depthStencil.sType =
    //         VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    //     depthStencil.depthTestEnable       = VK_TRUE;
    //     depthStencil.depthWriteEnable      = VK_TRUE;
    //     depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
    //     depthStencil.depthBoundsTestEnable = VK_FALSE;
    //     depthStencil.stencilTestEnable     = VK_FALSE;
    //
    //     VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    //     colorBlendAttachment.colorWriteMask =
    //         VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    //         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    //     colorBlendAttachment.blendEnable = VK_FALSE;
    //
    //     VkPipelineColorBlendStateCreateInfo colorBlending{};
    //     colorBlending.sType =
    //         VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    //     colorBlending.logicOpEnable     = VK_FALSE;
    //     colorBlending.logicOp           = VK_LOGIC_OP_COPY;
    //     colorBlending.attachmentCount   = 1;
    //     colorBlending.pAttachments      = &colorBlendAttachment;
    //     colorBlending.blendConstants[0] = 0.0f;
    //     colorBlending.blendConstants[1] = 0.0f;
    //     colorBlending.blendConstants[2] = 0.0f;
    //     colorBlending.blendConstants[3] = 0.0f;
    //
    //     VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
    //                                       VK_DYNAMIC_STATE_SCISSOR};
    //     VkPipelineDynamicStateCreateInfo dynamicState{};
    //     dynamicState.sType =
    //         VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    //     dynamicState.dynamicStateCount = ARRAY_COUNT(dynamicStates);
    //     dynamicState.pDynamicStates    = dynamicStates;
    //
    //     VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    //     pipelineLayoutInfo.sType =
    //         VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //     pipelineLayoutInfo.setLayoutCount = 1;
    //     pipelineLayoutInfo.pSetLayouts    = &s_Data.api.DescriptorSetLayout;
    //
    //     VK_CALL(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
    //                                    &s_Data.api.Pipeline.pipelineLayout));
    //     VkGraphicsPipelineCreateInfo pipelineInfo{};
    //     pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    //     pipelineInfo.stageCount          = 2;
    //     pipelineInfo.pStages             = shaderStages;
    //     pipelineInfo.pVertexInputState   = &vertexInputInfo;
    //     pipelineInfo.pInputAssemblyState = &inputAssembly;
    //     pipelineInfo.pViewportState      = &viewportState;
    //     pipelineInfo.pRasterizationState = &rasterizer;
    //     pipelineInfo.pMultisampleState   = &multisampling;
    //     pipelineInfo.pDepthStencilState  = &depthStencil;
    //     pipelineInfo.pColorBlendState    = &colorBlending;
    //     pipelineInfo.pDynamicState       = &dynamicState;
    //     pipelineInfo.layout              =
    //     s_Data.api.Pipeline.pipelineLayout; pipelineInfo.renderPass =
    //     Api::GetRenderpass(); pipelineInfo.subpass             = 0;
    //     pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
    //
    //     VK_CALL(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
    //                                       &pipelineInfo, nullptr,
    //                                       &s_Data.api.Pipeline.pipeline));
    //     return pipeline;
    // }
}  // namespace FooGame
