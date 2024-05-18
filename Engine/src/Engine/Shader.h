#pragma once
#include <vulkan/vulkan.h>
#include "../Defines.h"
namespace Engine
{
    enum class ShaderStage
    {
        VERTEX,
        FRAGMENT,
    };
    class Shader
    {
        public:
            Shader(const String& path, ShaderStage stage);
            ~Shader();
            VkShaderModule GetModule() const;
            ShaderStage GetShaderStage() const { return m_Stage; };
            VkShaderStageFlagBits GetType();
            VkPipelineShaderStageCreateInfo CreateInfo();

        private:
            ShaderStage m_Stage;
            std::string m_Path;
            VkShaderModule m_Module;
    };
}  // namespace Engine
