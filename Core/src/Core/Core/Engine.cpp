#include "Engine.h"
#include <GLFW/glfw3.h>
#include "../Backend/VulkanCheckResult.h"
#include "../Core/Base.h"
#include "../Graphics/Semaphore.h"
#include "../Core/Window.h"
#include "../Graphics/Api.h"
#include "../Graphics/Renderer2D.h"
#include "../Graphics/Renderer3D.h"
#include "../Graphics/Swapchain.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include "vulkan/vulkan_core.h"

namespace FooGame
{

    struct FrameData
    {
            u32 imageIndex   = 0;
            u32 currentFrame = 0;
            i32 fbWidth      = 0;
            i32 fbHeight     = 0;
    };
    struct EngineComponents
    {
            GLFWwindow* windowHandle;
            Swapchain* swapchain;
            List<Fence> inFlightFences;
            List<Semaphore> imageAvailableSemaphores;
            List<Semaphore> renderFinishedSemaphores;
            bool framebufferResized = false;

            struct Command
            {
                    List<VkCommandBuffer> commandBuffers;
                    VkCommandPool commandPool;
            };
            Command command;
    };
    FrameData frameData{};
    EngineComponents comps{};

    u32 Engine::GetCurrentFrame()
    {
        return frameData.currentFrame;
    }
    u32 Engine::GetImageIndex()
    {
        return frameData.imageIndex;
    }
    static void check_vk_result(VkResult err)
    {
        if (err == 0)
        {
            return;
        }
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
        {
            abort();
        }
    }

    VkFormat Engine::GetSwapchainImageFormat()
    {
        return comps.swapchain->GetImageFormat();
    }
    ImGui_ImplVulkanH_Window g_MainWindowData;
    static VkDescriptorPool g_ImguiPool = nullptr;
    VkCommandBuffer Engine::BeginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = comps.command.commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer{};
        vkAllocateCommandBuffers(Api::GetDevice()->GetDevice(), &allocInfo,
                                 &commandBuffer);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    bool Engine::OnWindowResized(WindowResizeEvent& event)
    {
        comps.framebufferResized = true;
        frameData.fbWidth        = event.GetWidth();
        frameData.fbHeight       = event.GetHeight();
        return true;
    }
    void Engine::EndSingleTimeCommands(VkCommandBuffer& commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        vkQueueSubmit(Api::GetDevice()->GetGraphicsQueue(), 1, &submitInfo,
                      VK_NULL_HANDLE);
        vkQueueWaitIdle(Api::GetDevice()->GetGraphicsQueue());

        vkFreeCommandBuffers(Api::GetDevice()->GetDevice(),
                             comps.command.commandPool, 1, &commandBuffer);
    }
    void Engine::Init(WindowsWindow& window)
    {
        comps.windowHandle = window.GetWindowHandle();
        Api::Init(&window);
        auto device = Api::GetDevice()->GetDevice();
        int w, h;
        glfwGetFramebufferSize(comps.windowHandle, &w, &h);
        comps.swapchain =
            SwapchainBuilder()
                .SetExtent({static_cast<uint32_t>(w), static_cast<uint32_t>(h)})
                .Build();

        Api::CreateRenderpass(GetSwapchainImageFormat());
        comps.swapchain->SetRenderpass();

        {
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = Api::GetDevice()->GetGraphicsFamily();

            VK_CALL(vkCreateCommandPool(device, &poolInfo, nullptr,
                                        &comps.command.commandPool));
        }

        comps.swapchain->CreateFramebuffers();
        {
            comps.command.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = comps.command.commandPool;
            allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount =
                (u32)comps.command.commandBuffers.size();
            VK_CALL(vkAllocateCommandBuffers(
                device, &allocInfo, comps.command.commandBuffers.data()));
        }
        {
            comps.imageAvailableSemaphores.clear();
            comps.renderFinishedSemaphores.clear();
            comps.inFlightFences.clear();

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                comps.imageAvailableSemaphores.emplace_back(Semaphore{device});
                comps.renderFinishedSemaphores.emplace_back(Semaphore{device});
                comps.inFlightFences.emplace_back(Fence{device});
            }
        }
        Renderer2D::Init();
        Renderer3D::Init();
        InitImgui();
    }
    Engine::~Engine()
    {
        Shutdown();
    }

    void Engine::BeginDrawing()
    {
        // frameData.DrawCall = 0;
        WaitFence(comps.inFlightFences[frameData.currentFrame]);
        u32 imageIndex;
        if (!AcquireNextImage(imageIndex))
        {
            frameData.imageIndex = imageIndex;
            return;
        }
        frameData.imageIndex = imageIndex;
        comps.inFlightFences[frameData.currentFrame].Reset();
        ResetCommandBuffer(
            comps.command.commandBuffers[frameData.currentFrame]);
        BeginDrawing_();
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }
    }
    VkExtent2D Engine::GetSwapchainExtent()
    {
        return comps.swapchain->GetExtent();
    }
    void Engine::EndDrawing()
    {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(
            ImGui::GetDrawData(),
            comps.command.commandBuffers[frameData.currentFrame]);
        Submit();
    }

    bool Engine::AcquireNextImage(u32& imageIndex)
    {
        auto err = comps.swapchain->AcquireNextImage(
            Api::GetDevice()->GetDevice(),
            comps.imageAvailableSemaphores[frameData.currentFrame],
            &imageIndex);

        if (err == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapchain();
            return false;
        }
        else if (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR)
        {
            std::cerr << "Something happened to swapchain" << std::endl;
        }

        return true;
    }
    void Engine::ResetCommandBuffer(VkCommandBuffer& buf,
                                    VkCommandBufferResetFlags flags)
    {
        vkResetCommandBuffer(buf, flags);
    }
    VkFramebuffer Engine::GetFramebuffer()
    {
        return comps.swapchain->GetFrameBuffer(frameData.imageIndex);
    }

    void Engine::BeginRenderpass()
    {
        auto cb = comps.command.commandBuffers[frameData.currentFrame];
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = Api::GetRenderpass();
        renderPassInfo.framebuffer =
            comps.swapchain->GetFrameBuffer(frameData.imageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = comps.swapchain->GetExtent();

        VkClearValue clearColor[2]     = {};
        clearColor[0]                  = {0.2f, 0.3f, 0.1f, 1.0f};
        clearColor[1]                  = {1.0f, 0};
        renderPassInfo.clearValueCount = ARRAY_COUNT(clearColor);
        renderPassInfo.pClearValues    = clearColor;

        vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }
    void Engine::BeginDrawing_()
    {
        auto cb = comps.command.commandBuffers[frameData.currentFrame];
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CALL(vkBeginCommandBuffer(cb, &beginInfo));
        BeginRenderpass();
    }
    VkCommandBuffer Engine::GetCurrentCommandbuffer()
    {
        return comps.command.commandBuffers[frameData.currentFrame];
    }
    void Engine::Submit()
    {
        auto cb = comps.command.commandBuffers[frameData.currentFrame];
        vkCmdEndRenderPass(cb);
        VK_CALL(vkEndCommandBuffer(cb));

        VkSemaphore waitSemaphores[] = {
            comps.imageAvailableSemaphores[frameData.currentFrame].Get()};
        VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores    = waitSemaphores;
        submitInfo.pWaitDstStageMask  = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers =
            &comps.command.commandBuffers[frameData.currentFrame];

        VkSemaphore signalSemaphores[] = {
            comps.renderFinishedSemaphores[frameData.currentFrame].Get()};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        VK_CALL(
            vkQueueSubmit(Api::GetDevice()->GetGraphicsQueue(), 1, &submitInfo,
                          comps.inFlightFences[frameData.currentFrame].Get()));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;

        VkSwapchainKHR swapChains[] = {*comps.swapchain->Get()};
        presentInfo.swapchainCount  = 1;
        presentInfo.pSwapchains     = swapChains;

        presentInfo.pImageIndices = &(frameData.imageIndex);

        auto result = vkQueuePresentKHR(Api::GetDevice()->GetPresentQueue(),
                                        &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            comps.framebufferResized)
        {
            comps.framebufferResized = false;
            RecreateSwapchain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        frameData.currentFrame =
            (frameData.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    void Engine::WaitFence(Fence& fence)
    {
        fence.Wait();
    }
    void Engine::RecreateSwapchain()
    {
        Api::WaitIdle();
        comps.swapchain->Recreate({static_cast<uint32_t>(frameData.fbWidth),
                                   static_cast<uint32_t>(frameData.fbHeight)});
    }
    void Engine::UpdateUniformData(UniformBufferObject ubo)
    {
        // comps.descriptor.UniformBuffers[GetCurrentFrame()]->SetData(sizeof(ubo),
        //                                                             &ubo);
    }
    void Engine::InitImgui()
    {
        auto device                       = Api::GetDevice();
        VkDescriptorPoolSize pool_sizes[] = {
            {               VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {         VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {         VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {  VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {  VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {      VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags   = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = ARRAY_COUNT(pool_sizes);
        pool_info.pPoolSizes    = pool_sizes;

        VK_CALL(vkCreateDescriptorPool(device->GetDevice(), &pool_info, nullptr,
                                       &g_ImguiPool));
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |=
            ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(comps.windowHandle, false);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance                  = Api::GetInstance();
        init_info.PhysicalDevice            = device->GetPhysicalDevice();
        init_info.Device                    = device->GetDevice();
        init_info.Queue                     = device->GetGraphicsQueue();
        init_info.QueueFamily               = device->GetGraphicsFamily();
        init_info.DescriptorPool            = g_ImguiPool;
        init_info.RenderPass                = Api::GetRenderpass();
        init_info.Subpass                   = 0;
        init_info.MinImageCount             = 2;
        init_info.ImageCount                = 2;
        init_info.CheckVkResultFn           = check_vk_result;
        init_info.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info);
        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void Engine::Shutdown()
    {
        auto device = Api::GetDevice()->GetDevice();
        Api::WaitIdle();
        vkDestroyCommandPool(device, comps.command.commandPool, nullptr);
        delete comps.swapchain;
        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            comps.imageAvailableSemaphores[i].Destroy(device);
            comps.renderFinishedSemaphores[i].Destroy(device);
            comps.inFlightFences[i].Destroy(device);
        }
        Renderer2D::Shutdown();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(Api::GetDevice()->GetDevice(), g_ImguiPool,
                                nullptr);

        Api::Shutdown();
    }
}  // namespace FooGame
