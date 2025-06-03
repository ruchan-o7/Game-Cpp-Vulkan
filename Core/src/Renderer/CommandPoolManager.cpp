#include "CommandPoolManager.h"
#include "../Core/Assert.h"

namespace fg {

CommandPoolManager::CommandPoolManager(const CreateInfo& info)
    : m_Device(info.Device),
      m_Name(std::move(info.Name)),
      m_QueueFamIndex(info.QueueFamilyIndex),
      m_Flags(info.Flags) {
}
CommandPoolManager::~CommandPoolManager() {
  FOO_ASSERT(m_CmdPools.size() == 0, "Command pools did not destroyed");
}

CommandPoolWrapper CommandPoolManager::AllocatePool(const char* debugName) {
  CommandPoolWrapper pool;
  if (!m_CmdPools.empty()) {
    pool = std::move(m_CmdPools.front());
    m_CmdPools.pop_front();
    m_Device.ResetCmdPool(pool);
  }
  if (pool == VK_NULL_HANDLE) {
    VkCommandPoolCreateInfo CmdPoolCI = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    CmdPoolCI.queueFamilyIndex = m_QueueFamIndex;
    CmdPoolCI.flags = m_Flags;

    pool = m_Device.CreateCommandPool(CmdPoolCI);
    FOO_ASSERT(pool != VK_NULL_HANDLE, "Failed to create Vulkan command pool");
  }
  return pool;
}
void CommandPoolManager::DestroyPools() {
  m_CmdPools.clear();
}
void CommandPoolManager::Recycle(CommandPoolWrapper&& pool) {
  m_CmdPools.emplace_back(std::move(pool));
}
}  // namespace fg
