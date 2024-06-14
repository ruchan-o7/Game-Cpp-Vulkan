#pragma once
#include "Types.h"
#include "VulkanLogicalDevice.h"
#include "Utils/VulkanObjectWrapper.h"

namespace ENGINE_NAMESPACE
{
    class RenderDevice;
    struct Texture2D;
    class VulkanBuffer
    {
            DELETE_COPY(VulkanBuffer);

        public:
            struct BuffData
            {
                    size_t Size = 0;
                    void* Data  = nullptr;
            };
            struct BuffDesc
            {
                    const char* Name = nullptr;
                    Vulkan::BUFFER_USAGE Usage;
                    Vulkan::BUFFER_MEMORY_FLAG MemoryFlag;
                    RenderDevice* pRenderDevice;
                    BuffData BufferData;
            };

            static std::unique_ptr<VulkanBuffer> CreateDynamicBuffer(size_t size,
                                                                     Vulkan::BUFFER_USAGE usage);
            static std::unique_ptr<VulkanBuffer> CreateVertexBuffer(
                const VulkanBuffer::BuffDesc& info);
            static std::unique_ptr<VulkanBuffer> CreateIndexBuffer(
                const VulkanBuffer::BuffDesc& info);

            VulkanBuffer(const BuffDesc& info);

            VulkanBuffer(VulkanBuffer&& other);
            ~VulkanBuffer();

            void UpdateData(void* data, size_t size, size_t offset = 0);
            void CopyTo(VulkanBuffer& destination, size_t size, VkCommandPool commandPool,
                        VkQueue queue);

            void MapMemory();
            void UnMapMemory();

            VkBuffer GetBuffer() const { return m_Buffer; };
            VkDeviceMemory& GetMemory();

        private:
            void Release();
            void Init();

        private:
            friend class RenderDevice;
            friend class VulkanDeviceContext;
            BuffDesc m_Desc;

            BufferWrapper m_Buffer;
            DeviceMemoryWrapper m_Memory;
            void* m_MappedPtr;
    };

}  // namespace ENGINE_NAMESPACE
