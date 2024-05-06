#pragma once
#include <vulkan/vulkan.h>
#include "pch.h"
namespace FooGame
{
    class Shader
    {
        public:
            explicit Shader(VkDevice device, std::string path);
            ~Shader();
            VkShaderModule GetModule() const;
            VkPipelineShaderStageCreateInfo CreateInfo(
                VkShaderStageFlagBits stage);

        private:
            VkDevice m_Device;
            std::string m_Path;
            VkShaderModule m_Module;
    };
}  // namespace FooGame
