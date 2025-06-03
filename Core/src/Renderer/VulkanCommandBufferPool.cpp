#include "VulkanCommandBufferPool.h"
#include "src/Core/Assert.h"
#include "src/Core/Log.h"

namespace fg {

VulkanCommandBufferPool::VulkanCommandBufferPool(std::shared_ptr<const VulkanLogicalDevice> logical,
                                                 uint32_t queueFamilyIndex,
                                                 VkCommandPoolCreateFlags flags)
    : m_Device(std::move(logical)), m_SupportedStagesMask(0), m_SupportedAccessMask(0) {
  VkCommandPoolCreateInfo info {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  info.queueFamilyIndex = queueFamilyIndex;
  info.flags = flags;
  m_CmdPool = m_Device->CreateCommandPool(info);
  FOO_ASSERT(m_CmdPool != VK_NULL_HANDLE);
}
VulkanCommandBufferPool::~VulkanCommandBufferPool() {
  for (auto cmd : m_CmdBuffers) {
    m_Device->FreeCmdBuffer(m_CmdPool, cmd);
  }
}

VkCommandBuffer VulkanCommandBufferPool::Get(const char* debugName) {
  VkCommandBuffer cmd = VK_NULL_HANDLE;
  if (!m_CmdBuffers.empty()) {
    cmd = m_CmdBuffers.front();
    auto err = vkResetCommandBuffer(cmd, 0);
    if (err != VK_SUCCESS) {
      FOO_CORE_ERROR("Failed to reset command buffer");
    }
    m_CmdBuffers.pop_front();
  }

  if (cmd == VK_NULL_HANDLE) {
    VkCommandBufferAllocateInfo info {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    info.commandPool = m_CmdPool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = 1;
    cmd = m_Device->AllocateCmdBuffer(info);
    if (cmd == VK_NULL_HANDLE) {
      FOO_CORE_ERROR("Can not create command buffer");
    }
  }
  VkCommandBufferBeginInfo info {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  auto err = vkBeginCommandBuffer(cmd, &info);
  if (err != VK_SUCCESS) {
    FOO_CORE_ERROR("Can not begin command buffer: {}", debugName != nullptr ? debugName : "");
  }

  return cmd;
}

void VulkanCommandBufferPool::Recycle(VkCommandBuffer&& cmd) {
  m_CmdBuffers.emplace_back(cmd);
  cmd = VK_NULL_HANDLE;
}

}  // namespace fg
