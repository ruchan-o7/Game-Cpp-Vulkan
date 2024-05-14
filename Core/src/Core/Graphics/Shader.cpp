#include "Shader.h"
#include <vulkan/vulkan.h>
#include "../File/FileHelper.h"
#include "Api.h"
namespace FooGame
{
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
            case FooGame::ShaderStage::VERTEX:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case FooGame::ShaderStage::FRAGMENT:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            default:
                std::cerr << "[ERROR] | Shader stage type not found !!!"
                          << std::endl;
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
