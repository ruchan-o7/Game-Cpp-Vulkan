#pragma once
#include <vulkan/vulkan.h>
#include "../Defines.h"
namespace Engine
{
    class Shader;

    struct Pipeline
    {
            VkPipeline pipeline;
            VkPipelineLayout pipelineLayout;
    };
    enum class CullMode
    {
        FRONT = BIT(0),
        BACK  = BIT(1),
        BOTH  = BIT(2)
    };
    enum class MultiSampling
    {
        LEVEL_1,
        LEVEL_2,
        LEVEL_4,
        LEVEL_8,
    };
    struct PipelineInfo
    {
            List<Shader*> Shaders;
            List<VkVertexInputBindingDescription> VertexBindings;
            List<VkVertexInputAttributeDescription> VertexAttributeDescriptons;
            CullMode CullMode;
            float LineWidth = 1.0f;
            MultiSampling MultiSampling;
            VkDescriptorSetLayout DescriptorSetLayout;
            u32 pushConstantSize  = 0;
            u32 pushConstantCount = 0;
    };
    Pipeline CreateGraphicsPipeline(PipelineInfo info);

}  // namespace Engine
