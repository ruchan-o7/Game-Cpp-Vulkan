#include "Engine.h"
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include "../Backend/VBackend.h"
#include "../Backend/Vertex.h"
#include "../Backend/VulkanCheckResult.h"
#include "../Core/Base.h"
#include "../Graphics/Buffer.h"
#include "../Graphics/Semaphore.h"

namespace FooGame
{
    static const List<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f},
         {1.0f, 0.0f, 0.0f, 1.0f},
         {1.0f, 1.0f},
         0.0f, 1.0f},
        { {0.5f, -0.5f, 0.0f},
         {0.0f, 1.0f, 0.0f, 1.0f},
         {1.0f, 1.0f},
         0.0f, 1.0f},
        {  {0.5f, 0.5f, 0.0f},
         {0.0f, 0.0f, 1.0f, 1.0f},
         {1.0f, 1.0f},
         0.0f, 1.0f},
        { {-0.5f, 0.5f, 0.0f},
         {1.0f, 1.0f, 1.0f, 1.0f},
         {1.0f, 1.0f},
         0.0f, 1.0f}
    };
    static const List<u32> indices = {0, 1, 2, 2, 3, 0};
    void Engine::Init(GLFWwindow* window)
    {
        m_WindowHandle = window;
        m_Api          = new Api;
        m_Api->Init(m_WindowHandle);
        int w, h;
        glfwGetFramebufferSize(m_WindowHandle, &w, &h);
        auto device  = m_Api->GetDevice();
        auto surface = m_Api->GetSurface();
        SwapchainBuilder scBuilder{*device, surface};
        m_Swapchain =
            scBuilder
                .SetExtent({static_cast<uint32_t>(w), static_cast<uint32_t>(h)})
                .Build();

        m_Api->CreateRenderpass(m_Swapchain->GetImageFormat());
        m_Swapchain->SetRenderpass(m_Api->GetRenderpass());
        m_Swapchain->CreateFramebuffers();
        m_Api->SetupDesciptorLayout();
        m_Api->CreateGraphicsPipeline();
        m_Api->CreateCommandPool();

        {
            BufferBuilder vBufBuilder{device->GetDevice()};
            vBufBuilder.SetUsage(BufferUsage::VERTEX)
                .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                .SetMemoryProperties(device->GetMemoryProperties())
                .SetInitialSize(sizeof(Vertex) * 1000);
            m_VertexBuffer =
                CreateShared<Buffer>(std::move(vBufBuilder.Build()));
            m_VertexBuffer->SetData(sizeof(Vertex) * vertices.size(),
                                    (void*)vertices.data());
            m_VertexBuffer->Bind();

            BufferBuilder iBufBuilder{device->GetDevice()};
            iBufBuilder.SetUsage(BufferUsage::INDEX)
                .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                .SetMemoryProperties(device->GetMemoryProperties())
                .SetInitialSize(sizeof(Vertex) * 1000);
            m_IndexBuffer =
                CreateShared<Buffer>(std::move(iBufBuilder.Build()));
            m_IndexBuffer->Bind();
            m_IndexBuffer->SetData(sizeof(indices), (void*)indices.data());

            m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        }
        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            BufferBuilder uBuffBuilder{device->GetDevice()};
            uBuffBuilder.SetUsage(BufferUsage::UNIFORM)
                .SetInitialSize(sizeof(UniformBufferObject))
                .SetMemoryProperties(device->GetMemoryProperties())
                .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            m_UniformBuffers[i] = CreateShared<Buffer>(uBuffBuilder.Build());
            m_UniformBuffers[i]->Bind();
        }

        m_Api->CreateDescriptorPool();
        {
            List<VkDescriptorSetLayout> layouts(
                MAX_FRAMES_IN_FLIGHT, m_Api->GetDescriptorSetLayout());
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool     = m_Api->GetDescriptorPool();
            allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
            allocInfo.pSetLayouts        = layouts.data();

            m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
            VK_CALL(vkAllocateDescriptorSets(device->GetDevice(), &allocInfo,
                                             m_DescriptorSets.data()));
        }

        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = *m_UniformBuffers[i]->GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range  = sizeof(UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet     = m_DescriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo     = &bufferInfo;

            vkUpdateDescriptorSets(device->GetDevice(), 1, &descriptorWrite, 0,
                                   nullptr);
        }
        {
            m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool        = m_Api->GetCommandPool();
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (u32)m_CommandBuffers.size();
            VK_CALL(vkAllocateCommandBuffers(device->GetDevice(), &allocInfo,
                                             m_CommandBuffers.data()));
        }
        {
            m_ImageAvailableSemaphores.clear();
            m_RenderFinishedSemaphores.clear();
            m_InFlightFences.clear();

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                m_ImageAvailableSemaphores.emplace_back(
                    Semaphore{device->GetDevice()});
                m_RenderFinishedSemaphores.emplace_back(
                    Semaphore{device->GetDevice()});
                m_InFlightFences.emplace_back(Fence{device->GetDevice()});
            }
        }
    }
    Engine::~Engine()
    {
        Shutdown();
    }
    void Engine::RunLoop()
    {
        while (!ShouldClose())
        {
            WaitFences();
            if (!AcquireNextImage())
            {
                continue;
            }
            UpdateUniforms();

            ResetFences();

            ResetCommandBuffers();

            Record();

            Submit();
        }
        m_Api->WaitIdle();
        Shutdown();
    }
    double deltaTime_     = 0;
    double lastFrameTime_ = 0;
    void Engine::UpdateUniforms()
    {
        double currentTime = glfwGetTime();
        deltaTime_         = currentTime - lastFrameTime_;
        lastFrameTime_     = currentTime;
        UniformBufferObject ubd{};

        // ubd.Model = glm::rotate(glm::mat4(1.0f),
        //                         (float)deltaTime_ * glm::radians(90.0f),
        //                         glm::vec3(0.0f, 0.0f, 1.0f));

        // ubd.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
        //                        glm::vec3(0.0f, 0.0f, 0.0f),
        //                        glm::vec3(0.0f, 0.0f, 1.0f));
        // ubd.Projection =
        //     glm::perspective(glm::radians(45.0f),
        //                      (float)m_Swapchain->GetExtent().width /
        //                          m_Swapchain->GetExtent().height,
        //                      0.1f, 1000.0f);
        ubd.Model             = glm::mat4{1.0f};
        ubd.View              = glm::mat4(1);
        ubd.Projection        = glm::mat4(1);
        ubd.Projection[1][1] *= -1;
        m_UniformBuffers[frameData.imageIndex]->SetData(sizeof(ubd), &ubd);
    }
    void Engine::Shutdown()
    {
        auto device = m_Api->GetDevice()->GetDevice();
        m_VertexBuffer->Release(device);
        m_IndexBuffer->Release(device);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_UniformBuffers[i]->Release(device);
            m_InFlightFences[i].Destroy(device);
            m_RenderFinishedSemaphores[i].Destroy(device);
            m_ImageAvailableSemaphores[i].Destroy(device);
        }

        m_Swapchain->Destroy();

        if (m_Api)
        {
            delete m_Api;
        }
    }
    bool Engine::AcquireNextImage()
    {
        auto err = m_Swapchain->AcquireNextImage(
            m_Api->GetDevice()->GetDevice(),
            m_ImageAvailableSemaphores[frameData.imageIndex],
            &frameData.imageIndex);

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
    void Engine::ResetCommandBuffers()
    {
        vkResetCommandBuffer(m_CommandBuffers[frameData.imageIndex], 0);
    }
    void Engine::Record()
    {
        auto cb = m_CommandBuffers[frameData.imageIndex];
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CALL(vkBeginCommandBuffer(cb, &beginInfo));

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = *m_Api->GetRenderpass();
        renderPassInfo.framebuffer =
            m_Swapchain->GetFrameBuffer(frameData.imageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_Swapchain->GetExtent();

        VkClearValue clearColor = {
            {0.2f, 0.3f, 0.1f, 1.0f}
        };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues    = &clearColor;

        vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_Api->GetPipeline().pipeline);
            VkViewport viewport{};
            viewport.x        = 0.0f;
            viewport.y        = 0.0f;
            viewport.width    = (float)m_Swapchain->GetExtent().width;
            viewport.height   = (float)m_Swapchain->GetExtent().height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(cb, 0, 1, &viewport);
            VkRect2D scissor{
                {0, 0},
                m_Swapchain->GetExtent()
            };
            vkCmdSetScissor(cb, 0, 1, &scissor);

            VkBuffer vertexBuffers[] = {*m_VertexBuffer->GetBuffer()};
            VkDeviceSize offsets[]   = {0};

            vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(cb, *m_IndexBuffer->GetBuffer(), 0,
                                 VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_Api->GetPipeline().pipelineLayout, 0, 1,
                                    &m_DescriptorSets[frameData.imageIndex], 0,
                                    nullptr);
            vkCmdDrawIndexed(cb, indices.size(), 1, 0, 0, 0);
        }
        vkCmdEndRenderPass(cb);
        VK_CALL(vkEndCommandBuffer(cb));
    }
    void Engine::ResetFences()
    {
        m_InFlightFences[frameData.imageIndex].Reset();
    }
    void Engine::Submit()
    {
        VkSemaphore waitSemaphores[] = {
            m_ImageAvailableSemaphores[frameData.imageIndex].Get()};
        VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores    = waitSemaphores;
        submitInfo.pWaitDstStageMask  = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &m_CommandBuffers[frameData.imageIndex];

        VkSemaphore signalSemaphores[] = {
            m_RenderFinishedSemaphores[frameData.imageIndex].Get()};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        VK_CALL(vkQueueSubmit(m_Api->GetDevice()->GetGraphicsQueue(), 1,
                              &submitInfo,
                              m_InFlightFences[frameData.imageIndex].Get()));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;

        VkSwapchainKHR swapChains[] = {*m_Swapchain->Get()};
        presentInfo.swapchainCount  = 1;
        presentInfo.pSwapchains     = swapChains;

        presentInfo.pImageIndices = &frameData.imageIndex;

        auto result = vkQueuePresentKHR(m_Api->GetDevice()->GetPresentQueue(),
                                        &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            RecreateSwapchain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        frameData.imageIndex =
            (frameData.imageIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    void Engine::WaitFences()
    {
        m_InFlightFences[frameData.imageIndex].Wait();
    }
    void Engine::RecreateSwapchain()
    {
        int w, h;
        glfwGetFramebufferSize(m_WindowHandle, &w, &h);
        m_Swapchain->Recreate(
            {static_cast<uint32_t>(w), static_cast<uint32_t>(h)});
    }

}  // namespace FooGame
