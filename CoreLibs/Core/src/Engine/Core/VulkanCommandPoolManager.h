#pragma once
#include <atomic>
#include <deque>
#include "Utils/IndexWrapper.h"
#include "Utils/VulkanObjectWrapper.h"
#include "VulkanLogicalDevice.h"

namespace ENGINE_NAMESPACE
{
    class VulkanCommandPoolManager
    {
        public:
            struct CreateInfo
            {
                    const VulkanLogicalDevice& LogicalDevice;
                    std::string Name;
                    const HardwareQueueIndex queueFamilyIndex;
                    const VkCommandPoolCreateFlags Flags;
            };
            DELETE_COPY_MOVE(VulkanCommandPoolManager);

            VulkanCommandPoolManager(const CreateInfo& ci);
            ~VulkanCommandPoolManager();

            CommandPoolWrapper AllocateCommandPool(const char* name = nullptr);

            void DestroyPools();
            int32_t GetAllocatedPoolCount() const { return m_AllocatedPoolCounter; }

            void RecycleCommandPool(CommandPoolWrapper&& cmdPool);

        private:
            const VulkanLogicalDevice& m_LogicalDevice;
            std::string m_Name;
            const HardwareQueueIndex m_QueueFamilyIndex;
            const VkCommandPoolCreateFlags m_CmdPoolFlags;

            std::deque<CommandPoolWrapper> m_CmdPools;

            std::atomic<int32_t> m_AllocatedPoolCounter{0};
    };

}  // namespace ENGINE_NAMESPACE
