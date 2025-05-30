#pragma once

#include "VulkanLogicalDevice.h"

namespace fg {

template <class T>
class VulkanObject {
  public:
    using VkObj = T;

    VulkanObject() : m_Device(nullptr), m_VulkanObject(VK_NULL_HANDLE) {
    }

    VulkanObject(VkObj&& object, const std::shared_ptr<const VulkanLogicalDevice>& device)
        : m_VulkanObject(object), m_Device(device) {
      object = VK_NULL_HANDLE;
    }

    // Does not take ownership
    explicit VulkanObject(VkObj obj) : m_VulkanObject(obj) {
    }

    VulkanObject(const VulkanObject&) = delete;
    VulkanObject(VulkanObject&) = delete;

    VulkanObject(VulkanObject&& rhs) noexcept
        : m_VulkanObject(rhs.m_VulkanObject), m_Device(std::move(rhs.m_Device)) {
      rhs.m_VulkanObject = VK_NULL_HANDLE;
    }

    VulkanObject& operator=(VulkanObject&& rhs) noexcept {
      Release();
      m_Device = std::move(rhs.m_Device);
      m_VulkanObject = rhs.m_VulkanObject;
      rhs.m_VulkanObject = VK_NULL_HANDLE;
      return *this;
    }
    operator VkObj() const {
      return m_VulkanObject;
    }
    const VkObj* operator&() const {
      return &m_VulkanObject;
    }

    void Release() {
      if (m_Device && m_VulkanObject != VK_NULL_HANDLE) {
        m_Device->DestroyObject(std::move(*this));
      }
      m_VulkanObject = VK_NULL_HANDLE;
      m_Device.reset();
    }

    ~VulkanObject() {
      Release();
    }

  private:
    std::shared_ptr<const VulkanLogicalDevice> m_Device;
    VkObj m_VulkanObject = VK_NULL_HANDLE;

    friend class VulkanLogicalDevice;
};

}  // namespace fg
