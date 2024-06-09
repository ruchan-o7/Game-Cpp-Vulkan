#pragma once
#include "RenderDevice.h"

namespace ENGINE_NAMESPACE
{
    class VulkanPipeline
    {
            DELETE_COPY(VulkanPipeline);

        public:
            struct CreateInfo
            {
                    std::vector<VkPipelineShaderStageCreateInfo> ShaderStages;
                    std::vector<VkVertexInputBindingDescription> VertexBindings;
                    std::vector<VkVertexInputAttributeDescription> VertexAttributes;
                    VkDescriptorSetLayout SetLayout;
                    RenderDevice* pRenderDevice;
                    VkRenderPass RenderPass;
                    CULL_MODE CullMode         = CULL_MODE_BACK;
                    uint8_t SampleCount        = 1;
                    float LineWidth            = 1.0f;
                    size_t PushConstantSize    = 0;
                    uint32_t PushConstantCount = 0;
            };
            VulkanPipeline(const CreateInfo& ci);

        private:
            CreateInfo m_Info;
            PipelineWrapper m_Pipeline;
            PipelineLayoutWrapper m_Layout;
    };

}  // namespace ENGINE_NAMESPACE
