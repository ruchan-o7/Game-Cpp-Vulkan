#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../Core/Base.h"
#include "../Graphics/Swapchain.h"
#include "Device.h"
struct GLFWwindow;
namespace FooGame
{
    struct GraphicsPipeline
    {
            VkPipeline pipeline;
            VkPipelineLayout pipelineLayout;
    };
    class Api
    {
        public:
            Api() = default;
            ~Api();
            void Init(GLFWwindow* window);
            VkSurfaceKHR GetSurface() const { return m_Surface; }
            void CreateRenderpass(VkFormat colorAttachmentFormat);
            void SetupDesciptorLayout();
            void CreateGraphicsPipeline();
            void CreateCommandPool();
            void CreateDescriptorPool();
            VkDescriptorPool GetDescriptorPool() const
            {
                return m_DescriptorPool;
            }
            Shared<Device> GetDevice() const { return m_Device; }
            VkCommandPool GetCommandPool() const { return m_CommandPool; }
            VkRenderPass* GetRenderpass() { return &m_RenderPass; }
            GraphicsPipeline GetPipeline() const { return m_GraphicsPipeline; }
            VkDescriptorSetLayout GetDescriptorSetLayout() const
            {
                return m_DescriptorSetLayout;
            }

        private:
            VkInstance m_Instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            Shared<Device> m_Device;
            VkSurfaceKHR m_Surface;
            VkRenderPass m_RenderPass;
            VkDescriptorSetLayout m_DescriptorSetLayout;
            GraphicsPipeline m_GraphicsPipeline;
            VkCommandPool m_CommandPool;
            VkDescriptorPool m_DescriptorPool;  // do this on higher level
    };

}  // namespace FooGame
