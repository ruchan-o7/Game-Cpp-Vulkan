#pragma once
#include <vulkan/vulkan.h>
#include "../Core/Base.h"
#include "../Graphics/Swapchain.h"
#include "Device.h"
#include "GLFW/glfw3.h"
struct GLFWwindow;
namespace FooGame
{

    struct UniformBufferObject
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
            static Api* Create(GLFWwindow* window);
            static Api* Get();
            void Init(GLFWwindow* window);
            void CreateRenderpass(VkFormat colorAttachmentFormat);
            void SetupDesciptorSetLayout();
            void CreateGraphicsPipeline();
            void CreateCommandPool();
            void CreateDescriptorPool();
            void Shutdown();
            void WaitIdle() { m_Device->WaitIdle(); }
            VkDescriptorPool GetDescriptorPool() const
            {
                return m_DescriptorPool;
            }
            VkSurfaceKHR GetSurface() const { return m_Surface; }
            Device* GetDevice() const { return m_Device.get(); }
            VkCommandPool GetCommandPool() const { return m_CommandPool; }
            VkRenderPass* GetRenderpass() { return &m_RenderPass; }
            GraphicsPipeline GetPipeline() const { return m_GraphicsPipeline; }
            VkInstance GetInstance() const { return m_Instance; }
            VkDescriptorSetLayout GetDescriptorSetLayout() const
            {
                return m_DescriptorSetLayout;
            }

        private:
            static Api* s_Instance;
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
