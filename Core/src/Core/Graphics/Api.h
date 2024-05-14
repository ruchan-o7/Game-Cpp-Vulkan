#pragma once
#include <vulkan/vulkan.h>
#include "../Core/Base.h"
#include "../Core/Window.h"
#include "Device.h"
#include "Pipeline.h"

namespace FooGame
{

    struct UniformBufferObject
    {
            alignas(16) glm::mat4 View;
            alignas(16) glm::mat4 Projection;
            alignas(16) glm::mat4 Reserved0;
            alignas(16) glm::mat4 Reserved1;
    };
    class Api
    {
        public:
            static void Init(WindowsWindow* window);
            static void CreateRenderpass(VkFormat colorAttachmentFormat);
            static void SetupDesciptorSetLayout();
            static void CreateCommandPool();
            static void Shutdown();
            static void WaitIdle();
            static VkSurfaceKHR GetSurface();
            static Device* GetDevice();
            static VkRenderPass GetRenderpass();
            static VkInstance GetInstance();
            static VkDescriptorSetLayout GetDescriptorSetLayout();

            static void DestoryBuffer(VkBuffer& buffer);
            static void FreeMemory(VkDeviceMemory& mem);
            static void AllocateMemory(const VkMemoryAllocateInfo& info,
                                       VkDeviceMemory& mem);
            static void BindBufferMemory(VkBuffer& buffer, VkDeviceMemory& mem,
                                         VkDeviceSize deviceSize = 0);
            static void UnMapMemory(VkDeviceMemory& mem);
            static void CmdCopyBuffer(VkCommandBuffer& cmd, VkBuffer& source,
                                      VkBuffer& target, u32 regionCount,
                                      VkBufferCopy& region);
            static void CreateBuffer(const VkBufferCreateInfo& info,
                                     VkBuffer& buffer);
            static void SetViewportAndScissors(VkCommandBuffer cmd, float w,
                                               float h);
            ~Api();

        private:
            Api() = default;
    };

}  // namespace FooGame
