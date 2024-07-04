#include "Shader.h"
#include <fstream>
#include <iostream>
#include <Log.h>
namespace FooGame
{
    static std::vector<char> ReadFile(const std::string& file_path)
    {
        std::ifstream file(file_path, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            // TODO: Handle without exception
            throw std::runtime_error("failed to open file!");
        }

        size_t file_size = (size_t)file.tellg();
        std::vector<char> buffer(file_size);

        file.seekg(0);
        file.read(buffer.data(), static_cast<std::streamsize>(file_size));

        file.close();

        return buffer;
    }
    Shader::Shader(const struct CreateInfo& ci) : m_Ci{ci}
    {
        auto code          = ReadFile(m_Ci.Path);
        auto logicalDevice = m_Ci.wpLogicalDevice.lock();

        VkShaderModuleCreateInfo create_info = {};
        create_info.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize                 = code.size();
        create_info.pCode                    = reinterpret_cast<const uint32_t*>(code.data());
        m_Module                             = logicalDevice->CreateShaderModule(create_info);
    }
    Shader::~Shader()
    {
        m_Module.Release();
    }
    VkShaderStageFlagBits Shader::GetType()
    {
        switch (m_Ci.Stage)
        {
            case ShaderStage::VERTEX:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::FRAGMENT:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            default:
                FOO_ENGINE_ERROR("Shader stage type not found");
                return VK_SHADER_STAGE_COMPUTE_BIT;
        }
    }
    VkPipelineShaderStageCreateInfo Shader::CreateInfo()
    {
        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage  = GetType();
        stageInfo.module = GetModule();
        stageInfo.pName  = "main";
        return stageInfo;
    }
}  // namespace FooGame
