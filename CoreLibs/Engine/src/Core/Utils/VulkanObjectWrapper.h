#pragma once
#include <cstdint>
#include <memory>
#include "../VulkanLogicalDevice.h"

namespace ENGINE_NAMESPACE
{
    enum class VulkanHandleTypeId : uint32_t;
    template <typename VulkanObjectType, VulkanHandleTypeId>
    class VulkanObjectWrapper
    {
        public:
            // clang-format off
            VulkanObjectWrapper() 
                : m_pLogicalDevice{nullptr},
                  m_VkObject{VK_NULL_HANDLE} {}
            // clang-format on
            VulkanObjectWrapper(std::shared_ptr<const VulkanLogicalDevice> pLogicalDevice,
                                VulkanObjectType&& vkObject)
                : m_pLogicalDevice{pLogicalDevice}, m_VkObject{vkObject}
            {
                vkObject = VK_NULL_HANDLE;
            }
            // This constructor does not take ownership of the vulkan object
            explicit VulkanObjectWrapper(VulkanObjectType vkObject) : m_VkObject{vkObject} {}
            DELETE_COPY(VulkanObjectWrapper);

            VulkanObjectWrapper(VulkanObjectWrapper&& rhs) noexcept
                : m_pLogicalDevice{std::move(rhs.m_pLogicalDevice)}, m_VkObject{rhs.m_VkObject}
            {
                rhs.m_VkObject = VK_NULL_HANDLE;
            }
            operator VulkanObjectType() const { return m_VkObject; }

            const VulkanObjectType* operator&() const { return &m_VkObject; }
            void Release()
            {
                // For externally managed objects, m_pLogicalDevice is null
                if (m_pLogicalDevice && m_VkObject != VK_NULL_HANDLE)
                {
                    m_pLogicalDevice->ReleaseVulkanObject(std::move(*this));
                }
                m_VkObject = VK_NULL_HANDLE;
                m_pLogicalDevice.reset();
            }

            ~VulkanObjectWrapper() { Release(); }

        private:
            friend class VulkanLogicalDevice;
            std::shared_ptr<const VulkanLogicalDevice> m_pLogicalDevice;
            VulkanObjectType m_VkObject;
    };
}  // namespace ENGINE_NAMESPACE
