#include "VulkanShader.h"
#include "../Core/Assert.h"
#include "VulkanLogicalDevice.h"
#include "../Core/Log.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fg {

VulkanShader::VulkanShader(const ShaderDescription& desc) {
  if (!std::filesystem::exists(desc.Path)) {
    auto formatted = fmt::format("Can not find: '{}' does not exists!", desc.Path.string());
    throw std::runtime_error(formatted.c_str());
  }
  std::ifstream in {desc.Path};
  size_t size = in.tellg();
  if (size == 0) {
    auto formatted = fmt::format("Failed to read: '{}' does not exists!", desc.Path.string());
    throw std::runtime_error(formatted.c_str());
  }

  ScopedBuffer byteCode {size};
  in.read((char*)byteCode.Data(), size);

  FOO_ASSERT(desc.Stage != 0);
  FOO_ASSERT(desc.ByteCode.Size != 0);
  m_Stage = desc.Stage;

  VkShaderModuleCreateInfo info {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  info.pCode = (uint32_t*)desc.ByteCode.Data;
  info.codeSize = desc.ByteCode.Size;
  m_Handle = desc.Device->CreateShader(info);
  if (m_Handle != VK_NULL_HANDLE) {
    FOO_CORE_ERROR("Can not create shader module '{}'", desc.Path.string());
  }
}

}  // namespace fg
