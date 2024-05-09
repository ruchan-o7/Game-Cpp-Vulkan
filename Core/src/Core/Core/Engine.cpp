#include "Engine.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include "../Backend/Vertex.h"
#include "../Backend/VulkanCheckResult.h"
#include "../Core/Base.h"
#include "../Graphics/Buffer.h"
#include "../Graphics/Semaphore.h"
#include "../Graphics/Image.h"
#include "../Core/PerspectiveCamera.h"
#include "vulkan/vulkan_core.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace FooGame
{
    static VkDescriptorPool g_ImguiPool = nullptr;
    Engine* Engine::s_Instance          = nullptr;
    Engine* Engine::Create(GLFWwindow* window)
    {
        s_Instance = new Engine{};
        s_Instance->Init(window);
        return s_Instance;
    }
    Device& Engine::GetDevice() const
    {
        return *m_Api->GetDevice();
    }
    VkCommandBuffer Engine::BeginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_Api->GetCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer{};
        vkAllocateCommandBuffers(m_Api->GetDevice()->GetDevice(), &allocInfo,
                                 &commandBuffer);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    bool Engine::OnWindowResized(WindowResizeEvent& event)
    {
        m_FramebufferResized = true;
        frameData.fbWidth    = event.GetWidth();
        frameData.fbHeight   = event.GetHeight();
        return true;
    }
    void Engine::EndSingleTimeCommands(VkCommandBuffer& commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        vkQueueSubmit(m_Api->GetDevice()->GetGraphicsQueue(), 1, &submitInfo,
                      VK_NULL_HANDLE);
        vkQueueWaitIdle(m_Api->GetDevice()->GetGraphicsQueue());

        vkFreeCommandBuffers(m_Api->GetDevice()->GetDevice(),
                             m_Api->GetCommandPool(), 1, &commandBuffer);
    }
    void Engine::Init(GLFWwindow* window)
    {
        m_WindowHandle = window;
        m_Api          = Api::Create(window);
        int w, h;
        glfwGetFramebufferSize(m_WindowHandle, &w, &h);
        auto device  = m_Api->GetDevice();
        auto surface = m_Api->GetSurface();
        SwapchainBuilder scBuilder{device, surface};
        m_Swapchain =
            scBuilder
                .SetExtent({static_cast<uint32_t>(w), static_cast<uint32_t>(h)})
                .Build();

        m_Api->CreateRenderpass(m_Swapchain->GetImageFormat());
        m_Swapchain->SetRenderpass(m_Api->GetRenderpass());
        m_Api->SetupDesciptorSetLayout();
        m_Api->CreateGraphicsPipeline();
        m_Api->CreateCommandPool();
        m_Swapchain->CreateFramebuffers();
        LoadTexture(m_Image, "../../../textures/texture.jpg");
        // texture sampler
        {
            auto props = device->GetPhysicalDeviceProperties();
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter    = VK_FILTER_LINEAR;
            samplerInfo.minFilter    = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy    = props.limits.maxSamplerAnisotropy;
            samplerInfo.borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable           = VK_FALSE;
            samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;

            VK_CALL(vkCreateSampler(device->GetDevice(), &samplerInfo, nullptr,
                                    &m_TextureSampler));
        }

        // buffers
        {
            auto builder =
                BufferBuilder()
                    .SetUsage(BufferUsage::VERTEX)
                    .SetInitialSize(sizeof(Vertex) * frameData.MaxVertices)
                    .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            m_VertexBuffer = CreateShared<Buffer>(builder.Build());
            m_VertexBuffer->Allocate();
            m_VertexBuffer->Bind();
            frameData.QuadVertexBufferBase = new Vertex[frameData.MaxVertices];
            u32 offset                     = 0;
            List<u32> quadIndices(frameData.MaxIndices);
            for (u32 i = 0; i < FrameData::MaxIndices; i += 6)
            {
                quadIndices[i + 0] = offset + 0;
                quadIndices[i + 1] = offset + 1;
                quadIndices[i + 2] = offset + 2;

                quadIndices[i + 3]  = offset + 2;
                quadIndices[i + 4]  = offset + 3;
                quadIndices[i + 5]  = offset + 0;
                offset             += 4;
            }
            // delete[] quadIndices;
            m_IndexBuffer =
                CreateShared<Buffer>(CreateIndexBuffer(quadIndices));
            quadIndices.clear();
            frameData.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
            frameData.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
            frameData.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
            frameData.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
        }
        m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            BufferBuilder uBuffBuilder{};
            uBuffBuilder.SetUsage(BufferUsage::UNIFORM)
                .SetInitialSize(sizeof(UniformBufferObject))
                .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            m_UniformBuffers[i] = CreateShared<Buffer>(uBuffBuilder.Build());
            m_UniformBuffers[i]->Allocate();
            m_UniformBuffers[i]->Bind();
        }
        m_Api->CreateDescriptorPool();
        // descriptor sets
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
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView   = m_Image.ImageView;
            imageInfo.sampler     = m_TextureSampler;

            VkWriteDescriptorSet descriptorWrites[2] = {};
            descriptorWrites[0].sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_DescriptorSets[i];
            descriptorWrites[0].dstBinding      = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType =
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo     = &bufferInfo;

            descriptorWrites[1].sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = m_DescriptorSets[i];
            descriptorWrites[1].dstBinding      = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo      = &imageInfo;

            vkUpdateDescriptorSets(device->GetDevice(),
                                   ARRAY_COUNT(descriptorWrites),
                                   descriptorWrites, 0, nullptr);
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
        InitImgui();
    }
    Engine::~Engine()
    {
        delete[] frameData.QuadVertexBufferBase;
        m_Api->WaitIdle();
        Shutdown();
    }
    void Engine::Close()
    {
        m_ShouldClose = true;
    }
    void Engine::Start()
    {
        ImGui::UpdateInputEvents(true);
        frameData.DrawCall = 0;
        WaitFences();
        u32 imageIndex;
        if (!AcquireNextImage(imageIndex))
        {
            frameData.imageIndex = imageIndex;
            return;
        }
        frameData.imageIndex = imageIndex;

        ResetFences();
        ResetCommandBuffers();
        BeginDrawing();
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::ShowDemoWindow();
        }
    }
    void Engine::End()
    {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(
            ImGui::GetDrawData(), m_CommandBuffers[frameData.currentFrame]);
        Submit();
    }

    double deltaTime_     = 0;
    double lastFrameTime_ = 0;
    void Engine::BeginScene(const PerspectiveCamera& camera)
    {
        UpdateUniforms(camera);
        StartBatch();
    }
    void Engine::StartBatch()
    {
        frameData.QuadIndexCount      = 0;
        frameData.QuadVertexBufferPtr = frameData.QuadVertexBufferBase;
    }
    void Engine::UpdateUniforms(const PerspectiveCamera& camera)
    {
        double currentTime = glfwGetTime();
        deltaTime_         = currentTime - lastFrameTime_;
        lastFrameTime_     = currentTime;
        UniformBufferObject ubd{};

        float aspect = (float)m_Swapchain->GetExtent().width /
                       (float)m_Swapchain->GetExtent().height;

        ubd.Model      = glm::mat4(1.0f);
        ubd.View       = camera.GetView();
        ubd.Projection = camera.GetProjection();

        m_UniformBuffers[frameData.currentFrame]->SetData(sizeof(ubd), &ubd);
    }
    void Engine::EndScene()
    {
        Flush();
    }
    void Engine::Flush()
    {
        auto cb = m_CommandBuffers[frameData.currentFrame];
        if (frameData.QuadIndexCount)
        {
            u32 dataSize = (u32)((uint8_t*)frameData.QuadVertexBufferPtr -
                                 (uint8_t*)frameData.QuadVertexBufferBase);
            m_VertexBuffer->SetData(dataSize, frameData.QuadVertexBufferBase);
            vkCmdDrawIndexed(cb, frameData.QuadIndexCount, 2, 0, 0, 0);
            frameData.DrawCall++;
        }
    }
    void Engine::Shutdown()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(m_Api->GetDevice()->GetDevice(), g_ImguiPool,
                                nullptr);
        auto device = m_Api->GetDevice()->GetDevice();
        m_VertexBuffer->Release();
        m_VertexBuffer.reset();
        m_IndexBuffer->Release();
        m_IndexBuffer.reset();
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_UniformBuffers[i]->Release();
            m_InFlightFences[i].Destroy(device);
            m_RenderFinishedSemaphores[i].Destroy(device);
            m_ImageAvailableSemaphores[i].Destroy(device);
        }
        DestroyImage(m_Image);
        vkDestroySampler(m_Api->GetDevice()->GetDevice(), m_TextureSampler,
                         nullptr);
        m_Swapchain->Destroy();

        if (m_Api)
        {
            delete m_Api;
        }
    }
    bool Engine::AcquireNextImage(u32& imageIndex)
    {
        auto err = m_Swapchain->AcquireNextImage(
            m_Api->GetDevice()->GetDevice(),
            m_ImageAvailableSemaphores[frameData.currentFrame], &imageIndex);

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
        vkResetCommandBuffer(m_CommandBuffers[frameData.currentFrame], 0);
    }

    void Engine::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                          const glm::vec4& color)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Engine::DrawQuad(const glm::vec3& position, const glm::vec2& size,
                          const glm::vec4& color)
    {
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), position) *
            glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        DrawQuad(transform, color);
    }

    void Engine::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
    {
        constexpr size_t quadVertexCount    = 4;
        constexpr glm::vec2 textureCoords[] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };
        if (frameData.QuadIndexCount >= FrameData::MaxIndices)
        {
            NextBatch();
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            frameData.QuadVertexBufferPtr->Position =
                transform * frameData.QuadVertexPositions[i];
            frameData.QuadVertexBufferPtr->Color = glm::vec3{1.0f, 1.0f, 1.0f};
            frameData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            frameData.QuadVertexBufferPtr++;
        }
        frameData.QuadIndexCount += 6;
        frameData.QuadCount++;
    }

    void Engine::DrawRotatedQuad(const glm::vec2& position,
                                 const glm::vec2& size, float rotation,
                                 const glm::vec4& color)
    {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
    }
    void Engine::DrawRotatedQuad(const glm::vec3& position,
                                 const glm::vec2& size, float rotation,
                                 const glm::vec4& color)
    {
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), position) *
            glm::rotate(glm::mat4(1.0f), glm::radians(rotation),
                        {0.0f, 0.0f, 1.0f}) *
            glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        DrawQuad(transform, color);
    }
    void Engine::BeginDrawing()
    {
        auto cb = m_CommandBuffers[frameData.currentFrame];
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

        VkClearValue clearColor[2]     = {};
        clearColor[0]                  = {0.2f, 0.3f, 0.1f, 1.0f};
        clearColor[1]                  = {1.0f, 0};
        renderPassInfo.clearValueCount = ARRAY_COUNT(clearColor);
        renderPassInfo.pClearValues    = clearColor;

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
                                    &m_DescriptorSets[frameData.currentFrame],
                                    0, nullptr);
        }
    }
    void Engine::NextBatch()
    {
        Flush();
        StartBatch();
    }
    void Engine::ResetFences()
    {
        m_InFlightFences[frameData.currentFrame].Reset();
    }
    void Engine::Submit()
    {
        auto cb = m_CommandBuffers[frameData.currentFrame];
        vkCmdEndRenderPass(cb);
        VK_CALL(vkEndCommandBuffer(cb));

        VkSemaphore waitSemaphores[] = {
            m_ImageAvailableSemaphores[frameData.currentFrame].Get()};
        VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores    = waitSemaphores;
        submitInfo.pWaitDstStageMask  = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffers[frameData.currentFrame];

        VkSemaphore signalSemaphores[] = {
            m_RenderFinishedSemaphores[frameData.currentFrame].Get()};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        VK_CALL(vkQueueSubmit(m_Api->GetDevice()->GetGraphicsQueue(), 1,
                              &submitInfo,
                              m_InFlightFences[frameData.currentFrame].Get()));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;

        VkSwapchainKHR swapChains[] = {*m_Swapchain->Get()};
        presentInfo.swapchainCount  = 1;
        presentInfo.pSwapchains     = swapChains;

        presentInfo.pImageIndices = &(frameData.imageIndex);

        auto result = vkQueuePresentKHR(m_Api->GetDevice()->GetPresentQueue(),
                                        &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            m_FramebufferResized)
        {
            m_FramebufferResized = false;
            RecreateSwapchain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        frameData.currentFrame =
            (frameData.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    void Engine::WaitFences()
    {
        m_InFlightFences[frameData.currentFrame].Wait();
    }
    void Engine::RecreateSwapchain()
    {
        m_Api->WaitIdle();
        m_Swapchain->Recreate({static_cast<uint32_t>(frameData.fbWidth),
                               static_cast<uint32_t>(frameData.fbHeight)});
    }
    void Engine::InitImgui()
    {
        auto device                       = m_Api->GetDevice();
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

        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(m_WindowHandle, false);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance                  = m_Api->GetInstance();
        init_info.PhysicalDevice            = device->GetPhysicalDevice();
        init_info.Device                    = device->GetDevice();
        init_info.Queue                     = device->GetGraphicsQueue();
        init_info.QueueFamily               = device->GetGraphicsFamily();
        init_info.DescriptorPool            = g_ImguiPool;
        init_info.RenderPass                = *m_Api->GetRenderpass();
        init_info.Subpass                   = 0;
        init_info.MinImageCount             = m_Swapchain->GetImageViewCount();
        init_info.ImageCount                = m_Swapchain->GetImageViewCount();
        init_info.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info);
    }

}  // namespace FooGame
