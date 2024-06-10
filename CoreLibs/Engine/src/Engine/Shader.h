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
                    class RenderDevice* pRenderDevice;
            };
            Shader(const CreateInfo& ci);
            ~Shader();
            VkShaderModule GetModule() const { return m_Module; };
            ShaderStage GetShaderStage() const { return m_Ci.Stage; };
            VkShaderStageFlagBits GetType();
            VkPipelineShaderStageCreateInfo CreateInfo();

        private:
            struct CreateInfo m_Ci;
            ShaderModuleWrapper m_Module;
    };
}  // namespace FooGame
