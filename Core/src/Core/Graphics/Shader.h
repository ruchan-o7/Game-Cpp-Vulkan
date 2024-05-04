#pragma once
#include <vulkan/vulkan.h>
#include <string>
namespace FooGame
{
    class Shader
    {
        public:
            explicit Shader(std::string path);
            ~Shader();
            VkShaderModule GetModule() const;

        private:
            std::string m_Path;
            VkShaderModule m_Module;
    };
}  // namespace FooGame
