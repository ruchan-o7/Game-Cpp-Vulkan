
#include "VulkanInstance.h"
#include "src/Renderer/VulkanDebug.h"
namespace fg {

VulkanInstance::~VulkanInstance() {
  FreeDebug(m_Instance);
  vkDestroyInstance(m_Instance, m_Allocator);
}
}  // namespace fg
