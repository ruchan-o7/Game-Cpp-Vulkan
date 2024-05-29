#include "Shader.h"
#include "Api.h"
#include <fstream>
#include <iostream>
#include "Device.h"
#include "src/Log.h"
#include "vulkan/vulkan_core.h"
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
    Shader::Shader(const std::string& path, ShaderStage stage)
        : m_Stage(stage), m_Path(path)
    {
        auto code = ReadFile(m_Path);

        VkShaderModuleCreateInfo create_info = {};
        create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode    = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;

        if (vkCreateShaderModule(Api::GetDevice()->GetDevice(), &create_info,
                                 nullptr, &m_Module) != VK_SUCCESS)
        {
            std::cerr << "Failed to load shader module of " << m_Path << '\n';
        }
    }
    VkShaderModule Shader::GetModule() const
    {
        return m_Module;
    }
    Shader::~Shader()
    {
        vkDestroyShaderModule(Api::GetDevice()->GetDevice(), m_Module, nullptr);
    }
    VkShaderStageFlagBits Shader::GetType()
    {
        switch (m_Stage)
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
