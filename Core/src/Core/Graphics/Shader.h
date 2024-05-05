#pragma once
#include <vulkan/vulkan.h>
#include "pch.h"
namespace FooGame
{
    class Shader
    {
        public:
            explicit Shader(std::string path);
            ~Shader();
            VkShaderModule GetModule() const;
            VkPipelineShaderStageCreateInfo CreateInfo(
                VkShaderStageFlagBits stage);

        private:
            std::string m_Path;
            VkShaderModule m_Module;
    };
}  // namespace FooGame
