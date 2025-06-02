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

    virtual void Release() {
      if (m_Device && m_VulkanObject != VK_NULL_HANDLE) {
        m_Device->DestroyObject(std::move(*this));
      }
      m_VulkanObject = VK_NULL_HANDLE;
      m_Device.reset();
    }

    virtual ~VulkanObject() {
      Release();
    }

  protected:
    std::shared_ptr<const VulkanLogicalDevice> m_Device;
    VkObj m_VulkanObject = VK_NULL_HANDLE;

    friend class VulkanLogicalDevice;
};

class VmaAllocationWrapper : public VulkanObject<VkBuffer> {
  public:
    VmaAllocationWrapper() : VulkanObject(), m_VmaAllocation(VK_NULL_HANDLE) {
    }

    VmaAllocationWrapper(VkBuffer&& handle, VmaAllocation&& allocation,
                         const std::shared_ptr<const VulkanLogicalDevice>& device)
        : VulkanObject(std::move(handle), device), m_VmaAllocation(std::move(allocation)) {
      handle = VK_NULL_HANDLE;
      allocation = VK_NULL_HANDLE;
    }

    // Does not take ownership
    explicit VmaAllocationWrapper(VkBuffer bufferHandle, VmaAllocation allocation)
        : VulkanObject(bufferHandle), m_VmaAllocation(allocation) {
    }

    VmaAllocationWrapper(VmaAllocationWrapper&& rhs) noexcept
        : VulkanObject(std::move(rhs.m_VulkanObject), std::move(rhs.m_Device)),
          m_VmaAllocation(std::move(rhs.m_VmaAllocation)) {
      rhs.m_VulkanObject = VK_NULL_HANDLE;
      rhs.m_VmaAllocation = VK_NULL_HANDLE;
    }

    VmaAllocationWrapper& operator=(VmaAllocationWrapper&& rhs) noexcept {
      Release();
      m_Device = std::move(rhs.m_Device);
      m_VulkanObject = rhs.m_VulkanObject;
      m_VmaAllocation = rhs.m_VmaAllocation;
      rhs.m_VulkanObject = VK_NULL_HANDLE;
      rhs.m_VmaAllocation = VK_NULL_HANDLE;
      return *this;
    }

    operator VkBuffer() const {
      return m_VulkanObject;
    }

    operator VmaAllocation() const {
      return m_VmaAllocation;
    }

    const VkBuffer* operator&() const {
      return &m_VulkanObject;
    }
    const VmaAllocation* VmaPtr() const {
      return &m_VmaAllocation;
    }
    virtual void Release() {
      if (m_Device && m_VulkanObject != VK_NULL_HANDLE || m_VmaAllocation != VK_NULL_HANDLE) {
        m_Device->DestroyObject(std::move(*this));
      }
      m_VulkanObject = VK_NULL_HANDLE;
      m_Device.reset();
    }

    operator bool() const {
      return m_VmaAllocation != nullptr && m_VulkanObject != VK_NULL_HANDLE;
    }

    virtual ~VmaAllocationWrapper() {
      Release();
    }

  private:
    VmaAllocation m_VmaAllocation;
    friend class VulkanLogicalDevice;
};

}  // namespace fg
