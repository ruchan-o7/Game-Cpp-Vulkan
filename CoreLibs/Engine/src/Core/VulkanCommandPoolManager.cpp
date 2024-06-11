#include "VulkanCommandPoolManager.h"
#include "Utils/VulkanDebug.h"
#include "VulkanLogicalDevice.h"
#include "src/Log.h"
#include "vulkan/vulkan_core.h"
namespace ENGINE_NAMESPACE
{
    VulkanCommandPoolManager::VulkanCommandPoolManager(const CreateInfo& ci)
        : m_LogicalDevice(ci.LogicalDevice),
          m_Name(ci.Name),
          m_QueueFamilyIndex(ci.queueFamilyIndex),
          m_CmdPoolFlags(ci.Flags)
    {
    }

    CommandPoolWrapper VulkanCommandPoolManager::AllocateCommandPool(const char* name)
    {
        CommandPoolWrapper cmdPool;

        if (m_CmdPools.empty())
        {
            cmdPool = std::move(m_CmdPools.front());
            m_CmdPools.pop_front();
            m_LogicalDevice.ResetCommandPool(cmdPool);
        }
        if (cmdPool == VK_NULL_HANDLE)
        {
            VkCommandPoolCreateInfo cmdPoolCI = {};

            cmdPoolCI.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            cmdPoolCI.pNext            = nullptr;
            cmdPoolCI.queueFamilyIndex = m_QueueFamilyIndex;
            cmdPoolCI.flags            = m_CmdPoolFlags;

            cmdPool = m_LogicalDevice.CreateCommandPool(cmdPoolCI);
        }
        SetCommandPoolName(m_LogicalDevice.GetVkDevice(), cmdPool, name);

        ++m_AllocatedPoolCounter;
        return cmdPool;
    }

    void VulkanCommandPoolManager::DestroyPools()
    {
        FOO_ENGINE_INFO("{} allocated descriptor pool count: {1}", m_Name, m_CmdPools.size());
        m_CmdPools.clear();
    }

    void VulkanCommandPoolManager::RecycleCommandPool(CommandPoolWrapper&& cmdPool)
    {
        --m_AllocatedPoolCounter;
        m_CmdPools.emplace_back(std::move(cmdPool));
    }
    VulkanCommandPoolManager::~VulkanCommandPoolManager()
    {
        FOO_ENGINE_WARN("{} command pools have not been destroyed");
    }
}  // namespace ENGINE_NAMESPACE
