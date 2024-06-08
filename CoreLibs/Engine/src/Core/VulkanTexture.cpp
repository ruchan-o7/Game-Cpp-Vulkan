#include "VulkanTexture.h"
#include <cassert>
namespace ENGINE_NAMESPACE
{
    VulkanTexture::VulkanTexture(const TextureDescription& desc, const TextureData& data)
        : m_Image(nullptr),
          m_Memory(nullptr),
          m_Sampler(nullptr),
          m_ImageView(nullptr),
          m_pLogicalDevice(data.LogicalDevice),
          m_Size(data.Size),
          m_pTextureData(data.Data)
    {
        assert(0 && "NOT IMPLEMENTED YET");
    }

}  // namespace ENGINE_NAMESPACE
