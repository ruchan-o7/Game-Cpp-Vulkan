#pragma once
#include "../Core/RenderDevice.h"
#include <string>
namespace FooGame
{
    enum class ShaderStage
    {
        VERTEX,
        FRAGMENT,
    };
    class Shader
    {
        public:
            struct CreateInfo
            {
                    std::string Path;
                    ShaderStage Stage;
                    std::weak_ptr<VulkanLogicalDevice> wpLogicalDevice;
            };
            Shader(const CreateInfo& ci);
            ~Shader();
            VkShaderModule GetModule() const { return m_Module; };
            ShaderStage GetShaderStage() const { return m_Ci.Stage; };
            VkShaderStageFlagBits GetType();
            VkPipelineShaderStageCreateInfo CreateInfo();
            VkDescriptorSetLayout GetLayout() const;

        private:
            struct CreateInfo m_Ci;
            ShaderModuleWrapper m_Module;
    };
}  // namespace FooGame
