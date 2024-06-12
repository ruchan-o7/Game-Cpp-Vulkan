#include "Backend.h"
#include <GLFW/glfw3.h>
#include "../Engine/VulkanCheckResult.h"
#include "../Defines.h"
#include "../Window/Window.h"
#include "../Events/ApplicationEvent.h"
#include "../Core/EngineFactory.h"
#include "../Core/RenderDevice.h"
#include "../Core/VulkanTexture.h"
#include "../Core/VulkanBuffer.h"
#include "../Core/VulkanCommandBuffer.h"
#include <vector>
#include "Api.h"
#include "../Core/VulkanRenderpass.h"
#include "Types/DeletionQueue.h"
#include "src/Log.h"
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
            bool framebufferResized = false;

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
        allocInfo.commandPool        = bContext.commandPool;
        allocInfo.commandBufferCount = 1;

        auto cmd = bContext.pRenderDevice->AllocateCommandBuffer(allocInfo);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);
        return cmd;
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
        auto queue                    = bContext.pRenderDevice->GetGraphicsQueue();
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        bContext.pRenderDevice->FreeCommandBuffer(bContext.commandPool, commandBuffer);
    }
    void Backend::WaitIdle()
    {
        bContext.pRenderDevice->WaitIdle();
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

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = 0;
        bContext.commandPool      = bContext.pRenderDevice->CreateCommandPool();

        bContext.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool        = bContext.commandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)bContext.commandBuffers.size();

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            bContext.commandBuffers[i] = bContext.pRenderDevice->AllocateCommandBuffer(allocInfo);
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

    void Backend::BeginDrawing()
    {
        auto res = bContext.pSwapchain->AcquireNextImage();
        if (res.Result != VK_SUCCESS)
        {
            FOO_ENGINE_ERROR("Failed to acquire next image");
        }
        frameData.imageIndex = res.ImageIndex;
        auto cb              = GetCurrentCommandbuffer();

        vkResetCommandBuffer(cb, 0);
        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        auto err   = vkBeginCommandBuffer(cb, &info);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass        = bContext.pRenderPass->GetRenderPass();
        renderPassInfo.framebuffer       = bContext.FrameBuffers[frameData.imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = bContext.pSwapchain->GetExtent();

        VkClearValue clearColor[2]     = {};
        clearColor[0]                  = {0.2f, 0.3f, 0.1f, 1.0f};
        clearColor[1]                  = {1.0f, 0};
        renderPassInfo.clearValueCount = ARRAY_COUNT(clearColor);
        renderPassInfo.pClearValues    = clearColor;

        vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    Backend::~Backend()
    {
        Shutdown();
    }
    void Backend::CopyBufferToImage(VulkanBuffer& source, VulkanTexture& destination)
    {
        auto cmd = BeginSingleTimeCommands();
        VkBufferImageCopy region{};
        region.imageSubresource.aspectMask = destination.GetAspect();
        region.imageSubresource.mipLevel   = 0;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = {destination.GetExtent().width, destination.GetExtent().height, 1};
        vkCmdCopyBufferToImage(cmd, source.GetBuffer(), destination.GetImage()->GetImageHandle(),
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        EndSingleTimeCommands(cmd);
    }

    void Backend::TransitionImageLayout(class VulkanImage* image, VkFormat format,
                                        VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        auto cmd = BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout                       = oldLayout;
        barrier.newLayout                       = newLayout;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.image                           = image->GetImageHandle();
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(cmd, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                             &barrier);

        EndSingleTimeCommands(cmd);
    }

    VkExtent2D Backend::GetSwapchainExtent()
    {
        return bContext.pSwapchain->GetExtent();
    }
    void Backend::EndDrawing()
    {
        Submit();
    }

    VkFramebuffer Backend::GetFramebuffer()
    {
        return bContext.FrameBuffers[bContext.pSwapchain->GetBackBufferIndex()];
    }

    void Backend::BeginRenderpass()
    {
        auto cb = GetCurrentCommandbuffer();
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass        = bContext.pRenderPass->GetRenderPass();
        renderPassInfo.framebuffer       = GetFramebuffer();
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = bContext.pSwapchain->GetExtent();

        VkClearValue clearColor[2]     = {};
        clearColor[0]                  = {0.2f, 0.3f, 0.1f, 1.0f};
        clearColor[1]                  = {1.0f, 0};
        renderPassInfo.clearValueCount = ARRAY_COUNT(clearColor);
        renderPassInfo.pClearValues    = clearColor;

        vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }
    VkCommandBuffer Backend::GetCurrentCommandbuffer()
    {
        return bContext.commandBuffers[frameData.currentFrame];
    }
    void Backend::Submit()
    {
        auto cb = GetCurrentCommandbuffer();
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb);
        vkCmdEndRenderPass(cb);
        VK_CALL(vkEndCommandBuffer(cb));
        VkResult res;
        res = bContext.pSwapchain->QueueSubmit(bContext.pRenderDevice->GetGraphicsQueue(),
                                               frameData.currentFrame, cb);

        if (res != VK_SUCCESS)
        {
            FOO_ENGINE_ERROR("Failed to submit draw command buffer");
        }

        auto result = bContext.pSwapchain->QueuePresent(bContext.pRenderDevice->GetGraphicsQueue());
        if (result.Result != VK_SUCCESS)
        {
            FOO_ENGINE_ERROR("Failed to present swap chain");
        }
        frameData.currentFrame = result.ImageIndex;
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
    }
}  // namespace FooGame
