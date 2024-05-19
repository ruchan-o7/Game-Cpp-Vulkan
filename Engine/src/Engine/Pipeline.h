#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "../Defines.h"
namespace FooGame
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
            std::vector<Shader*> Shaders;
            std::vector<VkVertexInputBindingDescription> VertexBindings;
            std::vector<VkVertexInputAttributeDescription>
                VertexAttributeDescriptons;
            CullMode CullMode;
            float LineWidth = 1.0f;
            MultiSampling MultiSampling;
            VkDescriptorSetLayout DescriptorSetLayout;
            uint32_t pushConstantSize  = 0;
            uint32_t pushConstantCount = 0;
    };
    Pipeline CreateGraphicsPipeline(PipelineInfo info);

}  // namespace FooGame
