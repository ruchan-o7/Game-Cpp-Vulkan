#pragma once
#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
namespace FooGame
{

    struct Init
    {
            vkb::Instance instance;
            vkb::InstanceDispatchTable inst_disp;
            VkSurfaceKHR surface;
            vkb::Device device;
            vkb::DispatchTable disp;
            vkb::Swapchain swapchain;
    };
    struct RenderData
    {
            VkQueue graphics_queue;
            VkQueue present_queue;

            std::vector<VkImage> swapchain_images;
            std::vector<VkImageView> swapchain_image_views;
            std::vector<VkFramebuffer> framebuffers;

            VkRenderPass render_pass;
            VkPipelineLayout pipeline_layout;
            VkPipeline graphics_pipeline;

            VkCommandPool command_pool;
            std::vector<VkCommandBuffer> command_buffers;

            std::vector<VkSemaphore> available_semaphores;
            std::vector<VkSemaphore> finished_semaphore;
            std::vector<VkFence> in_flight_fences;
            std::vector<VkFence> image_in_flight;
            size_t current_frame = 0;
            VkBuffer vertexBuffer;
            VkDeviceMemory vertexBufferMemory;
    };
    class Context
    {
        public:
            void Init();
            VkInstance GetInstance();
            VkDevice GetDevice();
            void DrawFrame();
            void ResizeSwapchain();
            void BeginDraw();
            void EndDraw();
            void DrawRectangle(glm::vec2 position, glm::vec2 size,
                               glm::vec3 color);
    };
}  // namespace FooGame
