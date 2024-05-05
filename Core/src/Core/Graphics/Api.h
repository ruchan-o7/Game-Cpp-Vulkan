#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../Core/Base.h"
#include "../Graphics/Swapchain.h"
#include "Core/Graphics/Buffer.h"
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
            void CreateSwapchain();
            void DestroySwapchain();
            void CreateRenderpass();
            void SetupDesciptorLayout();
            void CreateGraphicsPipeline();
            void CreateFramebuffers();
            void CreateCommandPool();
            void CreateDescriptorPool();

        private:
            VkInstance m_Instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            Shared<Device> m_Device;
            VkSurfaceKHR m_Surface;
            Swapchain m_Swapchain;
            VkRenderPass m_RenderPass;
            VkDescriptorSetLayout m_DescriptorSetLayout;
            GraphicsPipeline m_GraphicsPipeline;
            List<VkFramebuffer> m_SwapchainFrameBuffers;
            VkCommandPool m_CommandPool;
            VkDescriptorPool m_DescriptorPool;  // do this on higher level
    };

}  // namespace FooGame
