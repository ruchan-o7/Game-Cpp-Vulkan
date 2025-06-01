#pragma once
#include "Volk/volk.h"
#include "Misc.h"
#include "../Core/Buffer.h"
#include "../Core/Ref.h"
#include "../Renderer/VulkanObject.h"

namespace fg {

class Renderer;

struct ImageDescription {
    const char* Name = nullptr;
    VkImageType Type;
    VkFormat Format;
    uint32_t Width = 1, Height = 1, Depth = 1;
    // 0 Zero for full mip level generation
    uint32_t MipLevels = 1;
    VkImageUsageFlagBits Usage;
    VkImageLayout InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    bool Stage = false;
};

struct ImageViewDesc { };

class VulkanImage : public RefBase {
  public:
    VulkanImage(Renderer* renderer, const ImageDescription& desc, const Buffer buffer = Buffer());
    VkImageView CreateView();
    ~VulkanImage() = default;
    void SetState(ResourceState state) {
      m_State = state;
    }
    ResourceState State() const {
      return m_State;
    }

  private:
    ResourceState m_State = ResourceState::Unknown;
    Renderer* m_Renderer;
    ImageDescription m_Desc;
    ImageWrapper m_Image;
    VkDeviceMemory m_Memory;
};

}  // namespace fg
