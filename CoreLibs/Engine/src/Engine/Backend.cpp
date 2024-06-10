#include "Backend.h"
#include <GLFW/glfw3.h>
#include "../Engine/VulkanCheckResult.h"
#include "../Defines.h"
#include "../Window/Window.h"
#include "../Events/ApplicationEvent.h"
#include "../Core/EngineFactory.h"
#include "../Core/RenderDevice.h"
#include <vector>
#include "Api.h"
#include "Device.h"
#include "Swapchain.h"
#include "../Core/VulkanRenderpass.h"
#include <vulkan/vulkan.h>
#include "Types/DeletionQueue.h"
#include "vulkan/vulkan_core.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <Log.h>
namespace FooGame
{
    struct FrameStatistics
    {
            uint32_t imageIndex   = 0;
            uint32_t currentFrame = 0;
            int32_t fbWidth       = 0;
            int32_t fbHeight      = 0;
    };
    struct EngineComponents
    {
            GLFWwindow* windowHandle;
            // Swapchain* swapchain;
            std::vector<Fence> inFlightFences;
            std::vector<Semaphore> imageAvailableSemaphores;
            std::vector<Semaphore> renderFinishedSemaphores;
            bool framebufferResized = false;

            struct Command
            {
                    std::vector<VkCommandBuffer> commandBuffers;
                    VkCommandPool commandPool;
            };
            Command command;
            DeletionQueue deletionQueue;
    };
    struct BackendContext
    {
            RenderDevice* pRenderDevice         = nullptr;
            VulkanDeviceContext* pDeviceContext = nullptr;
            VulkanSwapchain* pSwapchain         = nullptr;
            VulkanRenderPass* pRenderPass       = nullptr;
            EngineCreateInfo engineCi;
            SwapchainDescription sDesc{};
            std::vector<FramebufferWrapper> FrameBuffers;
            CommandPoolWrapper commandPool;
            std::vector<VkCommandBuffer> commandBuffers;
    };
    BackendContext bContext{};
    FrameStatistics frameData{};
    EngineComponents comps{};

    uint32_t Backend::GetCurrentFrame()
    {
        return frameData.currentFrame;
    }
    uint32_t Backend::GetImageIndex()
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
    VkRenderPass Backend::GetRenderPass()
    {
        return bContext.pRenderPass->GetRenderPass();
    }
    VkFormat Backend::GetSwapchainImageFormat()
    {
        return bContext.pSwapchain->GetImageFormat();
        // return comps.swapchain->GetImageFormat();
    }
    ImGui_ImplVulkanH_Window g_MainWindowData;
    static VkDescriptorPool g_ImguiPool = nullptr;
    VkCommandBuffer Backend::BeginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool        = comps.command.commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer{};
        auto cmd = bContext.pRenderDevice->AllocateCommandBuffer(allocInfo);
        // vkAllocateCommandBuffers(Api::GetDevice()->GetDevice(), &allocInfo, &commandBuffer);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    bool Backend::OnWindowResized(WindowResizeEvent& event)
    {
        comps.framebufferResized = true;
        frameData.fbWidth        = event.GetWidth();
        frameData.fbHeight       = event.GetHeight();
        return true;
    }
    void Backend::EndSingleTimeCommands(VkCommandBuffer& commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        vkQueueSubmit(0, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(0);

        bContext.pRenderDevice->FreeCommandBuffer(bContext.commandPool, commandBuffer);
        // vkFreeCommandBuffers(Api::GetDevice()->GetDevice(), comps.command.commandPool, 1,
        //                      &commandBuffer);
    }
    void Backend::WaitIdle()
    {
        bContext.pRenderDevice->WaitIdle();
        // vkDeviceWaitIdle(Api::GetDevice()->GetDevice());
    }

    RenderDevice* Backend::GetRenderDevice()
    {
        return bContext.pRenderDevice;
    }
    void Backend::Init(Window& window)
    {
        FOO_ENGINE_INFO("Initializing renderer backend");
        comps.windowHandle = window.GetWindowHandle();
        auto factory       = EngineFactory::GetInstance();

        factory->CreateVulkanContexts(bContext.engineCi, &bContext.pRenderDevice,
                                      &bContext.pDeviceContext);
        factory->CreateSwapchain(bContext.pRenderDevice, bContext.pDeviceContext, bContext.sDesc,
                                 window.GetWindowHandle(), &bContext.pSwapchain);
        auto device = bContext.pRenderDevice->GetLogicalDevice()->GetVkDevice();

        RenderPassAttachmentDesc attachments[2];
        attachments[0].SampleCount    = 1;
        attachments[0].LoadOp         = ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].StoreOp        = ATTACHMENT_STORE_OP_STORE;
        attachments[0].Format         = TEX_FORMAT_RGBA8_UNORM;
        attachments[0].StencilLoadOp  = ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].StencilStoreOp = ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].InitialLayout  = RESOURCE_STATE_UNDEFINED;
        attachments[0].FinalLayout    = RESOURCE_STATE_PRESENT;

        attachments[1].Format         = TEX_FORMAT_D32_FLOAT;
        attachments[1].SampleCount    = 1;
        attachments[1].LoadOp         = ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].StoreOp        = ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].StencilLoadOp  = ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].StencilStoreOp = ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].InitialLayout  = RESOURCE_STATE_UNDEFINED;
        attachments[1].FinalLayout    = RESOURCE_STATE_DEPTH_WRITE;

        AttachmentReference colorAttachment{};
        colorAttachment.AttachmentIndex = 0;
        colorAttachment.State           = RESOURCE_STATE_RESOLVE_SOURCE;

        AttachmentReference depthAttachment{};
        depthAttachment.AttachmentIndex = 0;
        depthAttachment.State           = RESOURCE_STATE_RESOLVE_SOURCE;

        SubpassDesc subpasses[2];
        subpasses[0].pColorAttachment        = &colorAttachment;
        subpasses[0].pDepthStencilAttachment = &depthAttachment;

        RenderPassDesc desc{};
        desc.AttachmentCount = 2;
        desc.pAttachments    = attachments;
        desc.SubpassCount    = 1;
        desc.pSubpassDesc    = subpasses;
        bContext.pRenderDevice->CreateRenderPass(desc, &bContext.pRenderPass);

        bContext.FrameBuffers.resize(2);
        bContext.pRenderDevice->CreateFramebuffer(bContext.pSwapchain, bContext.FrameBuffers.data(),
                                                  bContext.pRenderPass);

        {
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = 0;  // Api::GetDevice()->GetGraphicsFamily();

            bContext.commandPool = bContext.pRenderDevice->CreateCommandPool();

            comps.deletionQueue.PushFunction(
                [&](VkDevice d) { vkDestroyCommandPool(d, comps.command.commandPool, nullptr); });
        }
        {
            bContext.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool        = bContext.commandPool;
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t)bContext.commandBuffers.size();

            for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                bContext.commandBuffers[i] =
                    bContext.pRenderDevice->AllocateCommandBuffer(allocInfo);
            }
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
            comps.deletionQueue.PushFunction(
                [&](VkDevice d)
                {
                    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                    {
                        comps.imageAvailableSemaphores[i].Destroy(d);
                        comps.renderFinishedSemaphores[i].Destroy(d);
                        comps.inFlightFences[i].Destroy(d);
                    }
                });
        }
        InitImgui();

        comps.deletionQueue.PushFunction(
            [&](VkDevice d)
            {
                ImGui_ImplVulkan_Shutdown();
                ImGui_ImplGlfw_Shutdown();
                ImGui::DestroyContext();

                vkDestroyDescriptorPool(d, g_ImguiPool, nullptr);
            });
    }
    Backend::~Backend()
    {
        Shutdown();
    }

    void Backend::BeginDrawing()
    {
        // frameData.DrawCall = 0;
        // WaitFence(comps.inFlightFences[frameData.currentFrame]);
        // uint32_t imageIndex;
        // if (!AcquireNextImage(imageIndex))
        // {
        //     frameData.imageIndex = imageIndex;
        //     return;
        // }
        // frameData.imageIndex = imageIndex;
        // comps.inFlightFences[frameData.currentFrame].Reset();
    }
    VkExtent2D Backend::GetSwapchainExtent()
    {
        return bContext.pSwapchain->GetExtent();
        // return comps.swapchain->GetExtent();
    }
    void Backend::EndDrawing()
    {
        Submit();
    }

    bool Backend::AcquireNextImage(uint32_t& imageIndex)
    {
        // auto err = comps.swapchain->AcquireNextImage(
        //     Api::GetDevice()->GetDevice(),
        //     comps.imageAvailableSemaphores[frameData.currentFrame], &imageIndex);
        //
        // if (err == VK_ERROR_OUT_OF_DATE_KHR)
        // {
        //     RecreateSwapchain();
        //     return false;
        // }
        // else if (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR)
        // {
        //     std::cerr << "Something happened to swapchain" << std::endl;
        // }
        //
        return true;
    }

    void Backend::ResetCommandBuffer(VkCommandBuffer& buf, VkCommandBufferResetFlags flags)
    {
        vkResetCommandBuffer(buf, flags);
    }
    VkFramebuffer Backend::GetFramebuffer()
    {
        return bContext.FrameBuffers[frameData.imageIndex];
        // return comps.swapchain->GetFrameBuffer(frameData.imageIndex);
    }

    void Backend::BeginRenderpass()
    {
        auto cb = comps.command.commandBuffers[frameData.currentFrame];
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = bContext.pRenderPass->GetRenderPass();  // Api::GetRenderpass();
        renderPassInfo.framebuffer       = bContext.FrameBuffers[frameData.imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = bContext.pSwapchain->GetExtent();

        VkClearValue clearColor[2]     = {};
        clearColor[0]                  = {0.2f, 0.3f, 0.1f, 1.0f};
        clearColor[1]                  = {1.0f, 0};
        renderPassInfo.clearValueCount = ARRAY_COUNT(clearColor);
        renderPassInfo.pClearValues    = clearColor;

        vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }
    void Backend::BeginDrawing_()
    {
        auto cb = comps.command.commandBuffers[frameData.currentFrame];
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CALL(vkBeginCommandBuffer(cb, &beginInfo));
        BeginRenderpass();
    }
    VkCommandBuffer Backend::GetCurrentCommandbuffer()
    {
        return comps.command.commandBuffers[frameData.currentFrame];
    }
    void Backend::Submit()
    {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                        comps.command.commandBuffers[frameData.currentFrame]);
        auto cb = comps.command.commandBuffers[frameData.currentFrame];
        vkCmdEndRenderPass(cb);
        VK_CALL(vkEndCommandBuffer(cb));
        bContext.pSwapchain->Present(frameData.currentFrame);
        // VkSemaphore waitSemaphores[] = {
        //     comps.imageAvailableSemaphores[frameData.currentFrame].Get()};
        // VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        // VkSubmitInfo submitInfo{};
        // submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        // submitInfo.waitSemaphoreCount = 1;
        // submitInfo.pWaitSemaphores    = waitSemaphores;
        // submitInfo.pWaitDstStageMask  = waitStages;
        //
        // submitInfo.commandBufferCount = 1;
        // submitInfo.pCommandBuffers    = &comps.command.commandBuffers[frameData.currentFrame];
        //
        // VkSemaphore signalSemaphores[] = {
        //     comps.renderFinishedSemaphores[frameData.currentFrame].Get()};
        // submitInfo.signalSemaphoreCount = 1;
        // submitInfo.pSignalSemaphores    = signalSemaphores;
        //
        // VK_CALL(vkQueueSubmit(Api::GetDevice()->GetGraphicsQueue(), 1, &submitInfo,
        //                       comps.inFlightFences[frameData.currentFrame].Get()));

        // VkPresentInfoKHR presentInfo{};
        // presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        //
        // presentInfo.waitSemaphoreCount = 1;
        // presentInfo.pWaitSemaphores    = signalSemaphores;
        //
        // VkSwapchainKHR swapChains[] = {*comps.swapchain->Get()};
        // presentInfo.swapchainCount  = 1;
        // presentInfo.pSwapchains     = swapChains;
        //
        // presentInfo.pImageIndices = &(frameData.imageIndex);
        //
        // auto result = vkQueuePresentKHR(Api::GetDevice()->GetPresentQueue(), &presentInfo);

        // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        //     comps.framebufferResized)
        // {
        //     comps.framebufferResized = false;
        //     RecreateSwapchain();
        // }
        // else if (result != VK_SUCCESS)
        // {
        //     throw std::runtime_error("failed to present swap chain image!");
        // }

        frameData.currentFrame = (frameData.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        ResetCommandBuffer(comps.command.commandBuffers[frameData.currentFrame]);
        BeginDrawing_();
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }
    }
    void Backend::WaitFence(Fence& fence)
    {
        fence.Wait();
    }
    void Backend::RecreateSwapchain()
    {
        bContext.pRenderDevice->WaitIdle();
        // Api::WaitIdle();
        bContext.pSwapchain->Resize(frameData.fbWidth, frameData.fbHeight);
    }
    void Backend::UpdateUniformData(UniformBufferObject ubo)
    {
        // comps.descriptor.UniformBuffers[GetCurrentFrame()]->SetData(sizeof(ubo),
        //                                                             &ubo);
    }
    void Backend::InitImgui()
    {
        auto device                       = bContext.pRenderDevice->GetVkDevice();
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
        pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets                    = 1000;
        pool_info.poolSizeCount              = ARRAY_COUNT(pool_sizes);
        pool_info.pPoolSizes                 = pool_sizes;

        VK_CALL(vkCreateDescriptorPool(bContext.pRenderDevice->GetVkDevice(), &pool_info, nullptr,
                                       &g_ImguiPool));
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io     = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(comps.windowHandle, false);

        auto queueProps = bContext.pRenderDevice->GetPhysicalDevice()->GetQueueProperties();

        auto graphics =
            bContext.pRenderDevice->GetPhysicalDevice()->FindQueueFamily(VK_QUEUE_GRAPHICS_BIT);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance                  = bContext.pRenderDevice->GetVkInstance();
        init_info.PhysicalDevice            = bContext.pRenderDevice->GetVkPhysicalDevice();
        init_info.Device                    = bContext.pRenderDevice->GetVkDevice();
        init_info.Queue       = bContext.pRenderDevice->GetLogicalDevice()->GetQueue(0, graphics);
        init_info.QueueFamily = graphics;
        init_info.DescriptorPool  = g_ImguiPool;
        init_info.RenderPass      = bContext.pRenderPass->GetRenderPass();
        init_info.Subpass         = 0;
        init_info.MinImageCount   = 2;
        init_info.ImageCount      = 2;
        init_info.CheckVkResultFn = check_vk_result;
        init_info.MSAASamples     = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info);
        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void Backend::Shutdown()
    {
        // auto device = Api::GetDevice()->GetDevice();
        // Api::WaitIdle();
        // comps.deletionQueue.Flush(device);
        //
        // Api::Shutdown();
    }
}  // namespace FooGame
