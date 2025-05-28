#include "Backend.h"
#include <GLFW/glfw3.h>
#include "../Engine/VulkanCheckResult.h"
#include "../../Core/Window.h"
#include "../../Events/ApplicationEvent.h"
#include "../Core/EngineFactory.h"
#include "../Core/RenderDevice.h"
#include "../Core/VulkanTexture.h"
#include "../Core/VulkanBuffer.h"
#include "../Core/VulkanCommandBuffer.h"
#include <vector>
#include "../Core/VulkanRenderpass.h"
#include "Types/DeletionQueue.h"
#include "src/Core/Application.h"
#include "src/Engine/Core/Types.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <Log.h>
namespace FooGame
{
    struct FrameStatistics
    {
            u32 currentFrame = 0;
            i32 fbWidth      = 0;
            i32 fbHeight     = 0;
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
            List<FramebufferWrapper> FrameBuffers;
            CommandPoolWrapper commandPool;
            Unique<vke::DescriptorAllocatorPool> DescriptorAllocatorPool;
            List<VkCommandBuffer> commandBuffers;
    };
    BackendContext bContext{};
    FrameStatistics frameData{};
    EngineComponents comps{};

    u32 Backend::GetCurrentFrame()
    {
        return frameData.currentFrame;
    }

    VkRenderPass Backend::GetRenderPass()
    {
        return bContext.pRenderPass->GetRenderPass();
    }
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
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
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

        auto queue = bContext.pRenderDevice->GetTransferQueue();
        vkQueueSubmit(queue, 1, &submitInfo, nullptr);
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
    void RecreateFramebuffer()
    {
        bContext.FrameBuffers.resize(2);
        bContext.pRenderDevice->CreateFramebuffer(bContext.pSwapchain, bContext.FrameBuffers.data(),
                                                  bContext.pRenderPass);
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
        attachments[0].Format         = TEX_FORMAT_RGBA8_UNORM_SRGB;
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
        RecreateFramebuffer();
        bContext.FrameBuffers.resize(2);
        bContext.pRenderDevice->CreateFramebuffer(bContext.pSwapchain, bContext.FrameBuffers.data(),
                                                  bContext.pRenderPass);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = 0;
        bContext.commandPool      = bContext.pRenderDevice->CreateCommandPool();

        bContext.commandBuffers.resize(2);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool        = bContext.commandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (u32)bContext.commandBuffers.size();

        for (int i = 0; i < 2; i++)
        {
            bContext.commandBuffers[i] = bContext.pRenderDevice->AllocateCommandBuffer(allocInfo);
        }

        bContext.DescriptorAllocatorPool = Unique<vke::DescriptorAllocatorPool>(
            vke::DescriptorAllocatorPool::Create(bContext.pRenderDevice->GetVkDevice()));
        bContext.DescriptorAllocatorPool->SetPoolSizeMultiplier(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                2);
        bContext.DescriptorAllocatorPool->SetPoolSizeMultiplier(VK_DESCRIPTOR_TYPE_SAMPLER, 2);
        bContext.DescriptorAllocatorPool->SetPoolSizeMultiplier(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                                2);
        BeginDrawing();
    }

    void Backend::BeginDrawing()
    {
        auto res = bContext.pSwapchain->AcquireNextImage();
        if (res.Result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateFramebuffer();
        }
        else if (res.Result != VK_SUCCESS)
        {
            FOO_ENGINE_ERROR("Failed to acquire next image");
        }
        auto cb = GetCurrentCommandbuffer();

        vkResetCommandBuffer(cb, 0);
        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        auto err   = vkBeginCommandBuffer(cb, &info);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass        = bContext.pRenderPass->GetRenderPass();
        renderPassInfo.framebuffer       = bContext.FrameBuffers[res.ImageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = bContext.pSwapchain->GetExtent();

        VkClearValue clearColor[2]     = {};
        clearColor[0]                  = {0.2f, 0.3f, 0.1f, 1.0f};
        clearColor[1]                  = {1.0f, 0};
        renderPassInfo.clearValueCount = ARRAY_COUNT(clearColor);
        renderPassInfo.pClearValues    = clearColor;

        vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
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

    void Backend::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                                        VkImageLayout newLayout)
    {
        auto cmd = BeginSingleTimeCommands();
        DEFER(EndSingleTimeCommands(cmd));

        VkImageMemoryBarrier barrier{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout                       = oldLayout;
        barrier.newLayout                       = newLayout;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.image                           = image;
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
            FOO_ENGINE_WARN("unsupported layout transition!");
            return;
        }

        vkCmdPipelineBarrier(cmd, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                             &barrier);
    }

    VkExtent2D Backend::GetSwapchainExtent()
    {
        return bContext.pSwapchain->GetExtent();
    }
    void Backend::SwapBuffers()
    {
        Submit();
        BeginDrawing();
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

    vke::DescriptorAllocatorHandle Backend::GetAllocatorHandle()
    {
        return bContext.DescriptorAllocatorPool->GetAllocator();
    }
    void Backend::Submit()
    {
        bContext.DescriptorAllocatorPool->Flip();
        auto cb = GetCurrentCommandbuffer();
        vkCmdEndRenderPass(cb);
        VK_CALL(vkEndCommandBuffer(cb));
        VkResult res;
        auto gq = bContext.pRenderDevice->GetGraphicsQueue();
        res     = bContext.pSwapchain->QueueSubmit(gq, cb);

        if (res != VK_SUCCESS)
        {
            FOO_ENGINE_ERROR("Failed to submit draw command buffer");
        }

        auto result = bContext.pSwapchain->QueuePresent(gq);
        if (result.Result == VK_ERROR_OUT_OF_DATE_KHR || result.Result == VK_SUBOPTIMAL_KHR)
        {
            RecreateFramebuffer();
        }
        else if (result.Result != VK_SUCCESS)
        {
            FOO_ENGINE_ERROR("Failed to present swap chain");
        }
        frameData.currentFrame = result.ImageIndex;
    }
    void Backend::SubmitToRenderThread(const std::function<void()>& f)
    {
        Application& app = Application::Get();
        app.SubmitToMainThread(f);
    }

    VkCommandPool Backend::GetCommandPool()
    {
        return bContext.commandPool;
    }

    void Backend::CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info,
                                            VkDescriptorSetLayout& layout)
    {
        VK_CALL(vkCreateDescriptorSetLayout(bContext.pRenderDevice->GetVkDevice(), &info, nullptr,
                                            &layout));
    }
    void Backend::BindVertexBuffers(u32 firstBinding, u32 bindingCount, VkBuffer* buffer,
                                    size_t* offsets)
    {
        vkCmdBindVertexBuffers(GetCurrentCommandbuffer(), firstBinding, bindingCount, buffer,
                               offsets);
    }
    void Backend::BindIndexBuffers(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
    {
        vkCmdBindIndexBuffer(GetCurrentCommandbuffer(), buffer, offset, indexType);
    }
    void Backend::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset,
                              u32 firstInstance)
    {
        vkCmdDrawIndexed(GetCurrentCommandbuffer(), indexCount, instanceCount, firstIndex,
                         vertexOffset, firstInstance);
    }
    void Backend::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
    {
        vkCmdDraw(GetCurrentCommandbuffer(), vertexCount, instanceCount, firstVertex,
                  firstInstance);
    }

    void Backend::BindGraphicPipeline(const VkPipeline& pipeline)
    {
        vkCmdBindPipeline(GetCurrentCommandbuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }
    void Backend::SetViewport(const VkViewport& viewport)
    {
        vkCmdSetViewport(GetCurrentCommandbuffer(), 0, 1, &viewport);
    }
    void Backend::SetScissor(const VkRect2D& scissor)
    {
        vkCmdSetScissor(GetCurrentCommandbuffer(), 0, 1, &scissor);
    }
    void Backend::PushConstant(const VkPipelineLayout& layout, VkShaderStageFlags stage, u32 offset,
                               u32 size, const void* data)
    {
        vkCmdPushConstants(GetCurrentCommandbuffer(), layout, stage, offset, size, data);
    }
    void Backend::UpdateDescriptorSets(i32 descriptorWriteCount,
                                       const VkWriteDescriptorSet* pDescriptorWrites,
                                       i32 descriptorCopyCount,
                                       const VkCopyDescriptorSet* pDescriptorCopies)
    {
        vkUpdateDescriptorSets(bContext.pRenderDevice->GetVkDevice(), descriptorWriteCount,
                               pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    }
    void Backend::BindGraphicPipelineDescriptorSets(VkPipelineLayout layout, u32 firstSet,
                                                    u32 descriptorSetCount,
                                                    const VkDescriptorSet* pDescriptorSets,
                                                    u32 dynamicOffsetCount,
                                                    const u32* pDynamicOffsets)
    {
        vkCmdBindDescriptorSets(GetCurrentCommandbuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
                                firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount,
                                pDynamicOffsets);
    }

    void Backend::Shutdown()
    {
    }
    Backend::~Backend()
    {
        Shutdown();
    }
}  // namespace FooGame
