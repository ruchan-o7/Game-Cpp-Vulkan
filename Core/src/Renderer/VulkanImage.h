#pragma once
#include "Misc.h"
#include "../Core/Buffer.h"
#include "../Core/Ref.h"
#include "../Renderer/VulkanObject.h"

namespace fg {

class Renderer;

enum class ImageUsage : uint8_t {
  None = 0,
  Depth,
  ColorAttachment,
  Staging,
  ShaderResource,
};

enum class ImageFormat {
  None,
  RGBA8,
  RGB8,
  RGBA16F,
  R8,
  R8Unsigned,
  D32,
};

enum class ImageType {
  None,
  Type1D,
  Type2D,
  Type3D,
};

struct ImageDescription {
    const char* Name = nullptr;
    ImageFormat Format = ImageFormat::None;
    ImageType Type = ImageType::None;
    ImageUsage Usage = ImageUsage::None;
    uint32_t Width = 0, Height = 0, Depth = 0;
    // 0 Zero for full mip level generation
    uint32_t MipLevels = 1;
};

struct ImageViewDesc { };

class VulkanImage : public RefBase {
  public:
    VulkanImage(Renderer* renderer, const ImageDescription& desc, const Buffer buffer = Buffer());
    ~VulkanImage() = default;

    VkImageView CreateView();

    void SetState(ResourceState state) {
      m_State = state;
    }

    ResourceState State() const {
      return m_State;
    }

    VkImage GetVkImage() const {
      return m_VmaImage;
    }

    uint32_t Width() const {
      return m_Desc.Width;
    }
    uint32_t Height() const {
      return m_Desc.Height;
    }
    uint32_t Depth() const {
      return m_Desc.Depth;
    }

  private:
    ResourceState m_State = ResourceState::Unknown;
    ImageDescription m_Desc;
    VmaImageWrapper m_VmaImage;
};

}  // namespace fg
