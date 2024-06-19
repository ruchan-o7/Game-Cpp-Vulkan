#pragma once
#include <memory>
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
                    std::weak_ptr<VulkanLogicalDevice> wpLogicalDevice;
                    VkRenderPass RenderPass;
                    CULL_MODE CullMode         = CULL_MODE_BACK;
                    uint8_t SampleCount        = 1;
                    float LineWidth            = 1.0f;
                    size_t PushConstantSize    = 0;
                    uint32_t PushConstantCount = 0;
                    const char* Name           = "Graphic pipeline";
            };
            VulkanPipeline(const CreateInfo& ci);
            VkPipelineLayout GetLayout() const { return m_Layout; }
            VkPipeline GetPipeline() const { return m_Pipeline; }
            VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; };
            ~VulkanPipeline();

        private:
            CreateInfo m_Info;
            PipelineWrapper m_Pipeline;
            PipelineLayoutWrapper m_Layout;
            DescriptorSetLayoutWrapper m_DescriptorSetLayout;
    };

}  // namespace ENGINE_NAMESPACE
