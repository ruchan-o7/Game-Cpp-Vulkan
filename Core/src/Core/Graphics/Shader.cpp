#include "Shader.h"
#include <vulkan/vulkan_core.h>
#include "../File/FileHelper.h"
#include "../Core/Renderer2D.h"
#include "pch.h"
namespace FooGame
{
    Shader::Shader(std::string path) : m_Path(std::move(path))
    {
        auto device = Renderer2D::GetDevice();
        auto code   = ReadFile(m_Path);

        VkShaderModuleCreateInfo create_info = {};
        create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode    = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;

        if (vkCreateShaderModule(device, &create_info, nullptr, &m_Module) !=
            VK_SUCCESS)
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
        auto device = Renderer2D::GetDevice();
        vkDestroyShaderModule(device, m_Module, nullptr);
    }
    VkPipelineShaderStageCreateInfo Shader::CreateInfo(
        VkShaderStageFlagBits stage)
    {
        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage  = stage;
        stageInfo.module = GetModule();
        stageInfo.pName  = "main";
        return stageInfo;
    }
}  // namespace FooGame
