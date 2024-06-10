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
            struct BuffDesc
            {
                    const char* name = nullptr;
                    Vulkan::BUFFER_USAGE Usage;
                    Vulkan::BUFFER_MEMORY_FLAG MemoryFlag;
                    RenderDevice* pRenderDevice;
                    std::weak_ptr<VulkanLogicalDevice> pLogicalDevice;
            };

            struct BuffData
            {
                    size_t Size = 0;
                    void* Data  = nullptr;
            };
            static std::unique_ptr<VulkanBuffer> CreateDynamicBuffer(size_t size,
                                                                     Vulkan::BUFFER_USAGE usage);
            static std::unique_ptr<VulkanBuffer> CreateVertexBuffer(
                VulkanBuffer::BuffData& data, RenderDevice* pRenderDevice,
                const char* name = "Vertex buffer");
            static std::unique_ptr<VulkanBuffer> CreateIndexBuffer(const BuffData& data);

            VulkanBuffer(const BuffDesc& info, const BuffData& data);

            VulkanBuffer(VulkanBuffer&& other);

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
            BuffData m_BufferData;

            BufferWrapper m_Buffer;
            DeviceMemoryWrapper m_Memory;
            void* m_MappedPtr;
    };

}  // namespace ENGINE_NAMESPACE
