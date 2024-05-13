#include "Pipeline.h"
#include "../Backend/VulkanCheckResult.h"
#include "Device.h"
#include "Api.h"
#include "vulkan/vulkan_core.h"
namespace FooGame
{

    Pipeline CreateGraphicsPipeline(PipelineInfo info)

    {
        assert(info.Shaders.size() != 0 &&
               "Check your pipeline info structure");
        Pipeline pipeline{};

        auto device = Api::GetDevice()->GetDevice();

        List<VkPipelineShaderStageCreateInfo> shaderStages;
        for (auto& shader : info.Shaders)
        {
            shaderStages.push_back(shader->CreateInfo());
        }
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        vertexInputInfo.vertexBindingDescriptionCount =
            static_cast<u32>(info.VertexBindings.size());
        vertexInputInfo.pVertexBindingDescriptions = info.VertexBindings.data();
        vertexInputInfo.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(info.VertexAttributeDescriptons.size());
        vertexInputInfo.pVertexAttributeDescriptions =
            info.VertexAttributeDescriptons.data();

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
        VkCullModeFlagBits cullMode{};
        switch (info.CullMode)
        {
            case CullMode::FRONT:
                cullMode = VK_CULL_MODE_FRONT_BIT;
                break;
            case CullMode::BACK:
                cullMode = VK_CULL_MODE_BACK_BIT;
                break;
            case CullMode::BOTH:
                cullMode = VK_CULL_MODE_FRONT_AND_BACK;
                break;
        }

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable        = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth               = info.LineWidth;
        rasterizer.cullMode                = cullMode;
        rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable         = VK_FALSE;
        VkSampleCountFlagBits sampleCount{};
        switch (info.MultiSampling)
        {
            case MultiSampling::LEVEL_1:
                sampleCount = VK_SAMPLE_COUNT_1_BIT;
                break;
            case MultiSampling::LEVEL_2:
                sampleCount = VK_SAMPLE_COUNT_2_BIT;
                break;
            case MultiSampling::LEVEL_4:
                sampleCount = VK_SAMPLE_COUNT_4_BIT;
                break;
            case MultiSampling::LEVEL_8:
                sampleCount = VK_SAMPLE_COUNT_8_BIT;
                break;
        }

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable  = VK_FALSE;
        multisampling.rasterizationSamples = sampleCount;

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
        pipelineLayoutInfo.pSetLayouts    = &info.DescriptorSetLayout;

        VkPushConstantRange pushConstants{};
        if (info.pushConstantCount > 0)
        {
            pushConstants.offset     = 0;
            pushConstants.size       = info.pushConstantSize;
            pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            pipelineLayoutInfo.pPushConstantRanges    = &pushConstants;
            pipelineLayoutInfo.pushConstantRangeCount = info.pushConstantCount;
        }

        VK_CALL(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                                       &pipeline.pipelineLayout));
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = shaderStages.data();
        pipelineInfo.pVertexInputState   = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState      = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState   = &multisampling;
        pipelineInfo.pDepthStencilState  = &depthStencil;
        pipelineInfo.pColorBlendState    = &colorBlending;
        pipelineInfo.pDynamicState       = &dynamicState;
        pipelineInfo.layout              = pipeline.pipelineLayout;
        pipelineInfo.renderPass          = Api::GetRenderpass();
        pipelineInfo.subpass             = 0;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

        VK_CALL(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                          &pipelineInfo, nullptr,
                                          &pipeline.pipeline));
        return pipeline;
    }

}  // namespace FooGame
