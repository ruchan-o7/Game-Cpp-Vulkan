#pragma once
#include <vulkan/vulkan.h>
#include "../Core/Base.h"
#include "../Graphics/Swapchain.h"
#include "Device.h"
#include <glm/glm.hpp>
struct GLFWwindow;
namespace FooGame
{

    struct UniformBufferData
    {
            alignas(16) glm::mat4 Model;
            alignas(16) glm::mat4 View;
            alignas(16) glm::mat4 Projection;
    };
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
            void Shutdown();
            void WaitIdle() { m_Device->WaitIdle(); }
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
