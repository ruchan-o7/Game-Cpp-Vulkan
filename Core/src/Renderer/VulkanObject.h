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

template <class TResource>
class VmaAllocationWrapper : public VulkanObject<TResource> {
    using ResourceType = TResource;

  public:
    VmaAllocationWrapper() : VulkanObject<TResource>(), m_VmaAllocation(VK_NULL_HANDLE) {
    }

    VmaAllocationWrapper(ResourceType&& handle, VmaAllocation&& allocation,
                         const std::shared_ptr<const VulkanLogicalDevice>& device)
        : VulkanObject<TResource>(std::move(handle), device),
          m_VmaAllocation(std::move(allocation)) {
      handle = VK_NULL_HANDLE;
      allocation = VK_NULL_HANDLE;
    }

    // Does not take ownership
    explicit VmaAllocationWrapper(ResourceType bufferHandle, VmaAllocation allocation)
        : VulkanObject<TResource>(bufferHandle), m_VmaAllocation(allocation) {
    }

    VmaAllocationWrapper(VmaAllocationWrapper&& rhs) noexcept
        : VulkanObject<TResource>(std::move(rhs.m_VulkanObject), std::move(rhs.m_Device)),
          m_VmaAllocation(std::move(rhs.m_VmaAllocation)) {
      rhs.m_VulkanObject = VK_NULL_HANDLE;
      rhs.m_VmaAllocation = VK_NULL_HANDLE;
    }

    VmaAllocationWrapper& operator=(VmaAllocationWrapper&& rhs) noexcept {
      Release();
      this->m_Device = std::move(rhs.m_Device);
      this->m_VulkanObject = rhs.m_VulkanObject;
      m_VmaAllocation = rhs.m_VmaAllocation;
      rhs.m_VulkanObject = VK_NULL_HANDLE;
      rhs.m_VmaAllocation = VK_NULL_HANDLE;
      return *this;
    }

    operator ResourceType() const {
      return this->m_VulkanObject;
    }

    operator VmaAllocation() const {
      return m_VmaAllocation;
    }

    const ResourceType* operator&() const {
      return &this->m_VulkanObject;
    }
    const VmaAllocation* VmaPtr() const {
      return &m_VmaAllocation;
    }
    virtual void Release() {
      if (this->m_Device && this->m_VulkanObject != VK_NULL_HANDLE ||
          m_VmaAllocation != VK_NULL_HANDLE) {
        this->m_Device->DestroyObject(std::move(*this));
      }
      this->m_VulkanObject = VK_NULL_HANDLE;
      this->m_Device.reset();
    }

    operator bool() const {
      return m_VmaAllocation != nullptr && this->m_VulkanObject != VK_NULL_HANDLE;
    }

    virtual ~VmaAllocationWrapper() {
      Release();
    }

  private:
    VmaAllocation m_VmaAllocation;
    friend class VulkanLogicalDevice;
};

}  // namespace fg
