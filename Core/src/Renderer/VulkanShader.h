#pragma once

#include "src/Core/Buffer.h"
#include "vulkan/vulkan_core.h"

namespace fg {

class VulkanLogicalDevice;

struct ShaderDescription {
    Buffer ByteCode;
    std::filesystem::path Path;
    VkShaderStageFlags Stage = 0;
    std::shared_ptr<VulkanLogicalDevice> Device;
};

class VulkanShader {
  public:
    VulkanShader(const ShaderDescription& desc);

    VkShaderModule GetHandle() const {
      return m_Handle;
    }
    VkShaderStageFlags GetStage() const {
      return m_Stage;
    }

  private:
    VkShaderModule m_Handle = VK_NULL_HANDLE;
    VkShaderStageFlags m_Stage = 0;
};
}  // namespace fg
