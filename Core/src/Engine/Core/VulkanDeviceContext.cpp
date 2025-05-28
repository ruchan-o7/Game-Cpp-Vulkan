#include "VulkanDeviceContext.h"
#include "RenderDevice.h"
#include "../Engine/Descriptor/DescriptorAllocator.h"
#include "VulkanBuffer.h"
#include "../Engine/Types/GraphicTypes.h"
#include <Log.h>
namespace ENGINE_NAMESPACE
{

    VulkanDeviceContext::VulkanDeviceContext(RenderDevice* pDevice,
                                             const EngineCreateInfo& engineCI,
                                             const DeviceContextDesc& desc)
        : m_pRenderDevice(pDevice),
          m_DevDesc(desc),
          m_EngineCI(engineCI),
          m_DescriptorAllocator(vke::DescriptorAllocatorPool::Create(pDevice->GetVkDevice()))
    {
        FOO_ENGINE_TRACE("Device context created");
        m_DescriptorAllocator->SetPoolSizeMultiplier(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
        {
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = 0;  // Api::GetDevice()->GetGraphicsFamily();
            m_CmdPool = m_pRenderDevice->GetLogicalDevice()->CreateCommandPool(poolInfo);
        }

        // for (uint32_t i = 0; i < 2; i++)
        // {
        //     VulkanBuffer::BuffDesc desc{};
        //     desc.pRenderDevice   = m_pRenderDevice;
        //     desc.Usage           = Vulkan::BUFFER_USAGE_UNIFORM;
        //     desc.MemoryFlag      = Vulkan::BUFFER_MEMORY_FLAG_CPU_VISIBLE;
        //     desc.Name            = "Uniform buffer";
        //     desc.BufferData.Data = {0};
        //     desc.BufferData.Size = sizeof(UniformBufferObject);

        //     m_UniformBuffers[i] = std::move(std::make_unique<VulkanBuffer>(desc));
        //     m_UniformBuffers[i]->MapMemory();
        // }
    }

    void VulkanDeviceContext::Draw()
    {
    }
    void VulkanDeviceContext::DrawIndexed()
    {
    }
    void VulkanDeviceContext::DrawMesh()
    {
    }
    void VulkanDeviceContext::IdleGPU()
    {
        m_pRenderDevice->WaitIdle();
    }
    VulkanDeviceContext::~VulkanDeviceContext()
    {
        Flush();
        FinishFrame();
    }

}  // namespace ENGINE_NAMESPACE
