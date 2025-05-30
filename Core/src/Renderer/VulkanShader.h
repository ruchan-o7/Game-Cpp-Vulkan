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
    VkShaderStageFlags Stage = 0;
};

class VulkanShader {
  public:
    VulkanShader(ShaderModuleWrapper&& module, VkShaderStageFlags stage)
        : m_Handle(std::move(module)), m_Stage(stage) {
    }

    VkShaderModule GetHandle() const {
      return m_Handle;
    }
    VkShaderStageFlags GetStage() const {
      return m_Stage;
    }

  private:
    ShaderModuleWrapper m_Handle;
    VkShaderStageFlags m_Stage = 0;
};
}  // namespace fg
