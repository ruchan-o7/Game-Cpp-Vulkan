#pragma once
#include <deque>
#include "VulkanObject.h"

namespace fg {
class CommandPoolManager {
  public:
    struct CreateInfo {
        const VulkanLogicalDevice& Device;
        std::string Name;
        uint32_t QueueFamilyIndex;
        const VkCommandPoolCreateFlags Flags;
    };
    CommandPoolManager(const CreateInfo& info);
    ~CommandPoolManager();

    CommandPoolManager(CommandPoolManager&&) = delete;
    CommandPoolManager(const CommandPoolManager&) = delete;
    CommandPoolManager& operator=(CommandPoolManager&&) = delete;
    CommandPoolManager& operator=(const CommandPoolManager&) = delete;

    CommandPoolWrapper AllocatePool(const char* debugName = nullptr);
    void DestroyPools();
    void Recycle(CommandPoolWrapper&& pool);

  private:
    const VulkanLogicalDevice& m_Device;
    std::string m_Name;
    uint32_t m_QueueFamIndex;
    std::deque<CommandPoolWrapper> m_CmdPools;
    const VkCommandPoolCreateFlags m_Flags;
};
}  // namespace fg
