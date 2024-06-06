#pragma once
#include "Types.h"
#include "VulkanLogicalDevice.h"
#include "Utils/VulkanObjectWrapper.h"

namespace ENGINE_NAMESPACE
{
    class RenderDevice;
    struct Texture2D;
    enum class Bind
    {
        VERTEX,
        INDEX,
        UNIFORM,
    };
    struct BuffDesc
    {
            VkMemoryPropertyFlags MemoryFlags;
            BufferUsage Usage;
            Bind bind;
            RenderDevice* pRenderDevice;
    };
    struct BuffData
    {
            size_t Size;
            void* Data;
    };
    class VulkanBuffer
    {
        public:
            static std::unique_ptr<VulkanBuffer> CreateDynamicBuffer(size_t size,
                                                                     BufferUsage usage);
            static std::unique_ptr<VulkanBuffer> CreateVertexBuffer(const BuffData& data);
            static std::unique_ptr<VulkanBuffer> CreateIndexBuffer(const BuffData& data);

            VulkanBuffer(const BuffDesc& info, const BuffData& data);

            VulkanBuffer(VulkanBuffer&& other);

            VkBuffer& GetBuffer() const;
            VkDeviceMemory& GetMemory();

        private:
            void Release();
            void Create(const BuffData& data);
            static VkBufferUsageFlags ParseBufferUsage(BufferUsage usage);

        private:
            friend class RenderDevice;
            friend class VulkanDeviceContext;
            RenderDevice* m_pRenderDevice;
            BufferWrapper m_Buffer;
            DeviceMemoryWrapper m_Memory;
            void* m_Data;
            size_t m_Size;
            VkMemoryPropertyFlags m_MemoryFlags;
            BufferUsage m_Usage;
    };

}  // namespace ENGINE_NAMESPACE
