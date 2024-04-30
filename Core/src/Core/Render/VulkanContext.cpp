#include "VulkanContext.h"
#include "GLFW/glfw3.h"
#include "../Window.h"
#include "../Render/Shader.h"
#include "../Render/VulkanCheckResult.h"
#include <iostream>
#define MAX_FRAMES_IN_FLIGHT 2
/// This is valid when executing from project level. For example run like this
/// ./build/Game/Debug/Game.exe
#define VERT_PATH "./Shaders/vert.spv"
#define FRAG_PATH "./Shaders/frag.spv"
namespace FooGame
{
    RenderData renderData{};
    Init init{};

    void CreateSwapchain()
    {
        vkb::SwapchainBuilder swapchain_builder{init.device};
        auto swap_ret =
            swapchain_builder.set_old_swapchain(init.swapchain).build();
        if (!swap_ret)
        {
            std::cerr << swap_ret.error().message() << " "
                      << swap_ret.vk_result() << "\n";
        }
        vkb::destroy_swapchain(init.swapchain);
        init.swapchain = swap_ret.value();
    }
    void CreateFramebuffers()
    {
        renderData.swapchain_images = init.swapchain.get_images().value();
        renderData.swapchain_image_views =
            init.swapchain.get_image_views().value();

        renderData.framebuffers.resize(renderData.swapchain_image_views.size());

        for (size_t i = 0; i < renderData.swapchain_image_views.size(); i++)
        {
            VkImageView attachments[] = {renderData.swapchain_image_views[i]};

            VkFramebufferCreateInfo framebuffer_info = {};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass      = renderData.render_pass;
            framebuffer_info.attachmentCount = 1;
            framebuffer_info.pAttachments    = attachments;
            framebuffer_info.width           = init.swapchain.extent.width;
            framebuffer_info.height          = init.swapchain.extent.height;
            framebuffer_info.layers          = 1;

            if (init.disp.createFramebuffer(&framebuffer_info, nullptr,
                                            &renderData.framebuffers[i]) !=
                VK_SUCCESS)
            {
                return;
            }
        }
    }
    void CreateCommandpool()
    {
        VkCommandPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex =
            init.device.get_queue_index(vkb::QueueType::graphics).value();

        if (init.disp.createCommandPool(&pool_info, nullptr,
                                        &renderData.command_pool) != VK_SUCCESS)
        {
            std::cerr << "failed to create command pool\n";
            return;
        }
    }
    void CreateCommandBuffers()
    {
        renderData.command_buffers.resize(renderData.framebuffers.size());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = renderData.command_pool;
        allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount =
            (uint32_t)renderData.command_buffers.size();

        if (init.disp.allocateCommandBuffers(
                &allocInfo, renderData.command_buffers.data()) != VK_SUCCESS)
        {
            return;
        }

        for (size_t i = 0; i < renderData.command_buffers.size(); i++)
        {
            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (init.disp.beginCommandBuffer(renderData.command_buffers[i],
                                             &begin_info) != VK_SUCCESS)
            {
                return;
            }

            VkRenderPassBeginInfo render_pass_info = {};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_info.renderPass        = renderData.render_pass;
            render_pass_info.framebuffer       = renderData.framebuffers[i];
            render_pass_info.renderArea.offset = {0, 0};
            render_pass_info.renderArea.extent = init.swapchain.extent;
            VkClearValue clearColor{{{0.0f, 0.0f, 0.0f, 1.0f}}};
            render_pass_info.clearValueCount = 1;
            render_pass_info.pClearValues    = &clearColor;

            VkViewport viewport = {};
            viewport.x          = 0.0f;
            viewport.y          = 0.0f;
            viewport.width      = (float)init.swapchain.extent.width;
            viewport.height     = (float)init.swapchain.extent.height;
            viewport.minDepth   = 0.0f;
            viewport.maxDepth   = 1.0f;

            VkRect2D scissor = {};
            scissor.offset   = {0, 0};
            scissor.extent   = init.swapchain.extent;

            init.disp.cmdSetViewport(renderData.command_buffers[i], 0, 1,
                                     &viewport);
            init.disp.cmdSetScissor(renderData.command_buffers[i], 0, 1,
                                    &scissor);

            init.disp.cmdBeginRenderPass(renderData.command_buffers[i],
                                         &render_pass_info,
                                         VK_SUBPASS_CONTENTS_INLINE);

            init.disp.cmdBindPipeline(renderData.command_buffers[i],
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      renderData.graphics_pipeline);

            init.disp.cmdDraw(renderData.command_buffers[i], 3, 1, 0, 0);

            init.disp.cmdEndRenderPass(renderData.command_buffers[i]);

            if (init.disp.endCommandBuffer(renderData.command_buffers[i]) !=
                VK_SUCCESS)
            {
                std::cerr << "failed to record command buffer\n";
                return;
            }
        }
    }
    void Context::Init()
    {
        GLFWwindow* window = WindowsWindow::Get().GetWindowHandle();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        // instance
        {
            vkb::InstanceBuilder instanceBuilder;
            auto instance_ret = instanceBuilder.use_default_debug_messenger()
                                    .request_validation_layers()
                                    .build();
            if (!instance_ret)
            {
                std::cerr << instance_ret.error().message() << '\n';
                return;
            }
            init.instance = instance_ret.value();
        }
        // surface
        {
            init.inst_disp = init.instance.make_table();
            VK_CALL(glfwCreateWindowSurface(init.instance, window, nullptr,
                                            &init.surface));
        }
        // device
        {
            vkb::PhysicalDeviceSelector phys_device_selector(init.instance);
            auto phys_device_ret =
                phys_device_selector.set_surface(init.surface).select();
            if (!phys_device_ret)
            {
                std::cerr << phys_device_ret.error().message() << '\n';
                return;
            }
            vkb::PhysicalDevice physical_device = phys_device_ret.value();
            vkb::DeviceBuilder device_builder{physical_device};
            auto device_ret = device_builder.build();
            if (!device_ret)
            {
                std::cerr << device_ret.error().message() << "\n";
                return;
            }
            init.device = device_ret.value();
            init.disp   = init.device.make_table();
        }
        // swapchain
        {
            vkb::SwapchainBuilder swapchain_builder{init.device};
            auto swap_ret =
                swapchain_builder.set_old_swapchain(init.swapchain).build();
            if (!swap_ret)
            {
                std::cerr << swap_ret.error().message() << " "
                          << swap_ret.vk_result() << "\n";
                return;
            }
            vkb::destroy_swapchain(init.swapchain);
            init.swapchain = swap_ret.value();
        }
        // queue
        {
            auto gq = init.device.get_queue(vkb::QueueType::graphics);
            if (!gq.has_value())
            {
                std::cerr << "failed to get graphics queue: "
                          << gq.error().message() << "\n";
                return;
            }
            renderData.graphics_queue = gq.value();

            auto pq = init.device.get_queue(vkb::QueueType::present);
            if (!pq.has_value())
            {
                std::cerr << "failed to get present queue: "
                          << pq.error().message() << "\n";
                return;
            }
            renderData.present_queue = pq.value();
        }
        // render pass
        {
            VkAttachmentDescription color_attachment = {};
            color_attachment.format         = init.swapchain.image_format;
            color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
            color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference color_attachment_ref = {};
            color_attachment_ref.attachment            = 0;
            color_attachment_ref.layout =
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments    = &color_attachment_ref;

            VkSubpassDependency dependency = {};
            dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass          = 0;
            dependency.srcStageMask =
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask =
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo render_pass_info = {};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            render_pass_info.attachmentCount = 1;
            render_pass_info.pAttachments    = &color_attachment;
            render_pass_info.subpassCount    = 1;
            render_pass_info.pSubpasses      = &subpass;
            render_pass_info.dependencyCount = 1;
            render_pass_info.pDependencies   = &dependency;

            if (init.disp.createRenderPass(&render_pass_info, nullptr,
                                           &renderData.render_pass) !=
                VK_SUCCESS)
            {
                std::cerr << "failed to create render pass\n";
                return;  // failed to create render pass!
            }
        }
        // graphic pipeline
        {
            Shader vert{VERT_PATH};
            Shader frag{FRAG_PATH};
            VkPipelineShaderStageCreateInfo vert_stage_info = {};
            vert_stage_info.sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
            vert_stage_info.module = vert.GetModule();
            vert_stage_info.pName  = "main";

            VkPipelineShaderStageCreateInfo frag_stage_info = {};
            frag_stage_info.sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_stage_info.module = frag.GetModule();
            frag_stage_info.pName  = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info,
                                                               frag_stage_info};

            VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
            vertex_input_info.sType =
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_info.vertexBindingDescriptionCount   = 0;
            vertex_input_info.vertexAttributeDescriptionCount = 0;

            VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
            input_assembly.sType =
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport = {};
            viewport.x          = 0.0f;
            viewport.y          = 0.0f;
            viewport.width      = (float)init.swapchain.extent.width;
            viewport.height     = (float)init.swapchain.extent.height;
            viewport.minDepth   = 0.0f;
            viewport.maxDepth   = 1.0f;

            VkRect2D scissor = {};
            scissor.offset   = {0, 0};
            scissor.extent   = init.swapchain.extent;

            VkPipelineViewportStateCreateInfo viewport_state = {};
            viewport_state.sType =
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state.viewportCount = 1;
            viewport_state.pViewports    = &viewport;
            viewport_state.scissorCount  = 1;
            viewport_state.pScissors     = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizer = {};
            rasterizer.sType =
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable        = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth               = 1.0f;
            rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable         = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling = {};
            multisampling.sType =
                VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable  = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
            colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo color_blending = {};
            color_blending.sType =
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blending.logicOpEnable     = VK_FALSE;
            color_blending.logicOp           = VK_LOGIC_OP_COPY;
            color_blending.attachmentCount   = 1;
            color_blending.pAttachments      = &colorBlendAttachment;
            color_blending.blendConstants[0] = 0.0f;
            color_blending.blendConstants[1] = 0.0f;
            color_blending.blendConstants[2] = 0.0f;
            color_blending.blendConstants[3] = 0.0f;

            VkPipelineLayoutCreateInfo pipeline_layout_info = {};
            pipeline_layout_info.sType =
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_info.setLayoutCount         = 0;
            pipeline_layout_info.pushConstantRangeCount = 0;

            if (init.disp.createPipelineLayout(&pipeline_layout_info, nullptr,
                                               &renderData.pipeline_layout) !=
                VK_SUCCESS)
            {
                std::cerr << "failed to create pipeline layout\n";
                return;
            }

            std::vector<VkDynamicState> dynamic_states = {
                VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

            VkPipelineDynamicStateCreateInfo dynamic_info = {};
            dynamic_info.sType =
                VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_info.dynamicStateCount =
                static_cast<uint32_t>(dynamic_states.size());
            dynamic_info.pDynamicStates = dynamic_states.data();

            VkGraphicsPipelineCreateInfo pipeline_info = {};
            pipeline_info.sType =
                VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.stageCount          = 2;
            pipeline_info.pStages             = shader_stages;
            pipeline_info.pVertexInputState   = &vertex_input_info;
            pipeline_info.pInputAssemblyState = &input_assembly;
            pipeline_info.pViewportState      = &viewport_state;
            pipeline_info.pRasterizationState = &rasterizer;
            pipeline_info.pMultisampleState   = &multisampling;
            pipeline_info.pColorBlendState    = &color_blending;
            pipeline_info.pDynamicState       = &dynamic_info;
            pipeline_info.layout              = renderData.pipeline_layout;
            pipeline_info.renderPass          = renderData.render_pass;
            pipeline_info.subpass             = 0;
            pipeline_info.basePipelineHandle  = VK_NULL_HANDLE;

            if (init.disp.createGraphicsPipelines(
                    VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                    &renderData.graphics_pipeline) != VK_SUCCESS)
            {
                std::cerr << "failed to create pipline\n";
                return;
            }
        }
        CreateFramebuffers();
        CreateCommandpool();
        CreateCommandBuffers();

        // SYNC objects
        {
            renderData.available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
            renderData.finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
            renderData.in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
            renderData.image_in_flight.resize(init.swapchain.image_count,
                                              VK_NULL_HANDLE);

            VkSemaphoreCreateInfo semaphore_info = {};
            semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fence_info = {};
            fence_info.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_info.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                if (init.disp.createSemaphore(
                        &semaphore_info, nullptr,
                        &renderData.available_semaphores[i]) != VK_SUCCESS ||
                    init.disp.createSemaphore(
                        &semaphore_info, nullptr,
                        &renderData.finished_semaphore[i]) != VK_SUCCESS ||
                    init.disp.createFence(&fence_info, nullptr,
                                          &renderData.in_flight_fences[i]) !=
                        VK_SUCCESS)
                {
                    std::cout << "failed to create sync objects\n";
                    return;
                    // a frame
                }
            }
        }
    }
    void Context::DrawFrame()
    {
        init.disp.waitForFences(
            1, &renderData.in_flight_fences[renderData.current_frame], VK_TRUE,
            UINT64_MAX);

        uint32_t image_index = 0;
        VkResult result      = init.disp.acquireNextImageKHR(
            init.swapchain, UINT64_MAX,
            renderData.available_semaphores[renderData.current_frame],
            VK_NULL_HANDLE, &image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            ResizeSwapchain();
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            std::cout << "failed to acquire swapchain image. Error " << result
                      << "\n";
            return;
        }

        if (renderData.image_in_flight[image_index] != VK_NULL_HANDLE)
        {
            init.disp.waitForFences(1, &renderData.image_in_flight[image_index],
                                    VK_TRUE, UINT64_MAX);
        }
        renderData.image_in_flight[image_index] =
            renderData.in_flight_fences[renderData.current_frame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore wait_semaphores[] = {
            renderData.available_semaphores[renderData.current_frame]};
        VkPipelineStageFlags wait_stages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores    = wait_semaphores;
        submitInfo.pWaitDstStageMask  = wait_stages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &renderData.command_buffers[image_index];

        VkSemaphore signal_semaphores[] = {
            renderData.finished_semaphore[renderData.current_frame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signal_semaphores;

        init.disp.resetFences(
            1, &renderData.in_flight_fences[renderData.current_frame]);

        if (init.disp.queueSubmit(
                renderData.graphics_queue, 1, &submitInfo,
                renderData.in_flight_fences[renderData.current_frame]) !=
            VK_SUCCESS)
        {
            std::cout << "failed to submit draw command buffer\n";
            return;  //"failed to submit draw command buffer
        }

        VkPresentInfoKHR present_info = {};
        present_info.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores    = signal_semaphores;

        VkSwapchainKHR swapChains[] = {init.swapchain};
        present_info.swapchainCount = 1;
        present_info.pSwapchains    = swapChains;

        present_info.pImageIndices = &image_index;

        result =
            init.disp.queuePresentKHR(renderData.present_queue, &present_info);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            ResizeSwapchain();
        }
        else if (result != VK_SUCCESS)
        {
            std::cerr << "failed to present swapchain image\n";
            return;
        }

        renderData.current_frame =
            (renderData.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
        return;
    }
    VkInstance Context::GetInstance()
    {
        return init.instance;
    }
    VkDevice Context::GetDevice()
    {
        return init.device;
    }

    void Context::BeginDraw()
    {
    }

    void Context::EndDraw()
    {
    }

    void Context::ResizeSwapchain()
    {
        init.disp.deviceWaitIdle();

        init.disp.destroyCommandPool(renderData.command_pool, nullptr);

        for (auto framebuffer : renderData.framebuffers)
        {
            init.disp.destroyFramebuffer(framebuffer, nullptr);
        }
        init.swapchain.destroy_image_views(renderData.swapchain_image_views);

        (CreateSwapchain());
        (CreateFramebuffers());
        (CreateCommandpool());
        (CreateCommandBuffers());
    }
}  // namespace FooGame
