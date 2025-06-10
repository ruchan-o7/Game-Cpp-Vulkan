#pragma once
#include "Misc.h"
#include "VulkanObject.h"

#include "../Core/Buffer.h"
#include "../Core/Ref.h"
#include "../Core/Assert.h"

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
  RGBA8Unorm,
  RGB8,
  RGB8Unorm,
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
enum class ImageViewType {
  DepthStencil,
  ReadOnlyDepthStencil,
  RenderTarget
};
struct ImageViewDesc {
    ImageViewType ViewType;
    ImageType Type;
    ImageFormat Format;
};
class VulkanImageView;

class VulkanImage : public RefBase {
  public:
    // For new image
    VulkanImage(ReferenceCounter* counter, Renderer* renderer, const ImageDescription& desc,
                const Buffer buffer = Buffer());

    VulkanImage(ReferenceCounter* counter, Renderer* renderer, const ImageDescription& desc,
                ResourceState initialState, VkImage imageHandle);
    virtual ~VulkanImage() = default;

    void SetState(ResourceState state) {
      m_State = state;
    }

    bool IsInKnownState() const {
      return m_State != ResourceState::Unknown;
    }
    bool CheckState(ResourceState state) {
      FOO_ASSERT(IsInKnownState(), "Texture state is unknown");
      return m_State == state;
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
    const ImageDescription& GetDesc() const {
      return m_Desc;
    }
    Ref<VulkanImageView> GetDefaultView() {
      return m_DefaultView;
    }
    Ref<VulkanImageView> CreateView(const ImageViewDesc& desc);

  private:
    void CreateDefaultViews();

  private:
    Renderer* m_Renderer;
    ResourceState m_State = ResourceState::Unknown;
    ImageDescription m_Desc;
    VmaImageWrapper m_VmaImage;
    Ref<VulkanImageView> m_DefaultView;
};
class VulkanImageView : public RefBase {
  public:
    VulkanImageView(ReferenceCounter* counter, Renderer* renderer, const ImageViewDesc& desc,
                    ImageViewWrapper&& view, VulkanImage* pImage);
    VulkanImageView(ReferenceCounter* counter, Renderer* renderer, const ImageViewDesc& desc,
                    ImageViewWrapper&& view, Ref<VulkanImage> pImage);

    virtual ~VulkanImageView();

    VulkanImage const* GetImage() const {
      return m_BaseImage;
    }
    VulkanImage* GetImage() {
      if (m_BaseImage == nullptr) {
        FOO_ASSERT(m_sBaseImage != nullptr);
        return m_sBaseImage.get();
      }
      return m_BaseImage;
    }
    VkImageView GetHandle() const {
      return m_View;
    }

  private:
    Renderer* m_Renderer;
    ImageViewDesc m_Desc;
    ImageViewWrapper m_View;
    VulkanImage* m_BaseImage = nullptr;
    // Strong ref to image for preventing destroying images that created with non-default
    Ref<VulkanImage> m_sBaseImage;
};

}  // namespace fg
