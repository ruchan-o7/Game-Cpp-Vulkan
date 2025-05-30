#pragma once

#include "src/Core/Buffer.h"
#include "vulkan/vulkan_core.h"
#include "VulkanObject.h"

namespace fg {

class VulkanLogicalDevice;

struct ShaderDescription {
    Buffer ByteCode;
    std::filesystem::path Path;
    const char* Name = nullptr;
    std::string EntryPoint;
    VkShaderStageFlagBits Stage;
};

class VulkanShader {
  public:
    VulkanShader(const ShaderDescription& desc, ShaderModuleWrapper&& module,
                 VkShaderStageFlagBits stage)
        : m_Desc(desc), m_Handle(std::move(module)), m_Stage(stage) {
    }

    VkShaderModule GetHandle() const {
      return m_Handle;
    }
    VkShaderStageFlagBits GetStage() const {
      return m_Stage;
    }
    const char* EntryPoint() const {
      return m_Desc.EntryPoint.c_str();
    }
    const ShaderDescription& Desc() const {
      return m_Desc;
    }

  private:
    ShaderDescription m_Desc;
    ShaderModuleWrapper m_Handle;
    VkShaderStageFlagBits m_Stage;
};
}  // namespace fg
