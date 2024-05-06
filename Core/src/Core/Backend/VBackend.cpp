// #include "VBackend.h"
// #include <vulkan/vulkan_core.h>
// #include <cstring>
// #include "Core/Core/Renderer2D.h"
// #include "GLFW/glfw3.h"
// #include "../Core/Window.h"
// #include "../Graphics/Shader.h"
// #include "VulkanCheckResult.h"
// #include "../Backend/Vertex.h"
// #include "pch.h"
// #include "../Graphics/Buffer.h"
//
// #define VS_CODE
// #ifndef VS_CODE
// /// This is valid when executing from project level. For example run like
// this
// /// ./build/Game/Debug/Game.exe
// #define VERT_PATH "./Shaders/vert.spv"
// #define FRAG_PATH "./Shaders/frag.spv"
// #else
// #define VERT_PATH \
//     "C:\\Users\\jcead\\dev\\CppProjects\\game_1\\Shaders\\vert.spv"
// #define FRAG_PATH \
//     "C:\\Users\\jcead\\dev\\CppProjects\\game_1\\Shaders\\frag.spv"
// #endif
// namespace FooGame
// {
//
//     struct Init
//     {
//             vkb::Instance instance;
//             vkb::InstanceDispatchTable inst_disp;
//             VkSurfaceKHR surface;
//             vkb::Device device;
//             vkb::DispatchTable disp;
//             vkb::Swapchain swapchain;
//     };
//     struct ApiComponents
//     {
//             VkQueue graphics_queue;
//             VkQueue present_queue;
//
//             List<VkFramebuffer> framebuffers;
//
//             VkRenderPass render_pass;
//             VkPipelineLayout pipeline_layout;
//             VkPipeline graphics_pipeline;
//
//             VkDescriptorSetLayout descriptorSetLayout;
//             VkDescriptorPool descriptorPool;
//             List<VkDescriptorSet> descriptorSets;
//
//             VkCommandPool command_pool;
//             List<VkCommandBuffer> command_buffers;
//
//             List<VkSemaphore> available_semaphores;
//             List<VkSemaphore> finished_semaphore;
//             List<VkFence> in_flight_fences;
//             List<VkFence> image_in_flight;
//             size_t current_frame = 0;
//
//             VkBuffer* vertexBuffer;
//             VkBuffer* indexBuffer;
//
//             List<Buffer> uniformBuffers;
//
//             VkClearValue clearValue = {{{0.2f, 0.3f, 0.4f, 1.0f}}};
//             u32 imageIndex          = 0;
//     };
//
//     struct CameraData
//     {
//             alignas(16) glm::mat4 Model;
//             alignas(16) glm::mat4 View;
//             alignas(16) glm::mat4 Projection;
//     };
//     ApiComponents apiComps{};
//     Init init{};
//     static inline u32 findMemoryType(u32 filter,
//                                      VkMemoryPropertyFlags properties)
//     {
//         VkPhysicalDeviceMemoryProperties memProperties;
//         vkGetPhysicalDeviceMemoryProperties(init.device.physical_device,
//                                             &memProperties);
//
//         for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
//         {
//             if ((filter & (1 << i)) &&
//                 (memProperties.memoryTypes[i].propertyFlags & properties)
//                 ==
//                     properties)
//             {
//                 return i;
//             }
//         }
//         return 0;
//     }
//     u32 API::GetBackBufferIndex() const
//     {
//         return apiComps.current_frame;
//     }
//     VkPhysicalDeviceMemoryProperties API::GetMemoryProperties()
//     {
//         VkPhysicalDeviceMemoryProperties props;
//         vkGetPhysicalDeviceMemoryProperties(init.device.physical_device,
//                                             &props);
//         return props;
//     }
//
//     void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
//                       VkMemoryPropertyFlags memoryFlags, VkBuffer&
//                       buffer, VkDeviceMemory& memory)
//     {
//         auto device = init.device;
//         VkBufferCreateInfo bufferInfo{};
//         bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//         bufferInfo.size        = size;
//         bufferInfo.usage       = usage;
//         bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//         if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) !=
//         VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to create buffer!");
//         }
//
//         VkMemoryRequirements memRequirements;
//         vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
//
//         VkMemoryAllocateInfo allocInfo{};
//         allocInfo.sType          =
//         VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; allocInfo.allocationSize
//         = memRequirements.size; allocInfo.memoryTypeIndex =
//             findMemoryType(memRequirements.memoryTypeBits, memoryFlags);
//
//         VK_CALL(vkAllocateMemory(device, &allocInfo, nullptr, &memory));
//
//         vkBindBufferMemory(device, buffer, memory, 0);
//     }
//
//     void CreateSwapchain()
//     {
//         vkb::SwapchainBuilder swapchain_builder{init.device};
//         auto swap_ret =
//             swapchain_builder.set_old_swapchain(init.swapchain).build();
//         if (!swap_ret)
//         {
//             std::cerr << swap_ret.error().message() << " "
//                       << swap_ret.vk_result() << "\n";
//         }
//         vkb::destroy_swapchain(init.swapchain);
//         init.swapchain = swap_ret.value();
//     }
//     void CreateFramebuffers()
//     {
//         auto imageViews = init.swapchain.get_image_views().value();
//
//         apiComps.framebuffers.resize(imageViews.size());
//
//         for (size_t i = 0; i < imageViews.size(); i++)
//         {
//             VkImageView attachments[] = {imageViews[i]};
//
//             VkFramebufferCreateInfo framebuffer_info = {};
//             framebuffer_info.sType =
//             VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//             framebuffer_info.renderPass      = apiComps.render_pass;
//             framebuffer_info.attachmentCount = 1;
//             framebuffer_info.pAttachments    = attachments;
//             framebuffer_info.width           =
//             init.swapchain.extent.width; framebuffer_info.height =
//             init.swapchain.extent.height; framebuffer_info.layers = 1;
//
//             VK_CALL(init.disp.createFramebuffer(&framebuffer_info,
//             nullptr,
//                                                 &apiComps.framebuffers[i]));
//         }
//     }
//     void CreateCommandpool()
//     {
//         VkCommandPoolCreateInfo pool_info = {};
//         pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//         pool_info.flags =
//         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
//
//         pool_info.queueFamilyIndex =
//             init.device.get_queue_index(vkb::QueueType::graphics).value();
//
//         VK_CALL(init.disp.createCommandPool(&pool_info, nullptr,
//                                             &apiComps.command_pool));
//     }
//     void CreateCommandBuffers()
//     {
//         apiComps.command_buffers.resize(apiComps.framebuffers.size());
//
//         VkCommandBufferAllocateInfo allocInfo = {};
//         allocInfo.sType       =
//         VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//         allocInfo.commandPool = apiComps.command_pool;
//         allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//         allocInfo.commandBufferCount =
//             (uint32_t)apiComps.command_buffers.size();
//
//         VK_CALL(init.disp.allocateCommandBuffers(
//             &allocInfo, apiComps.command_buffers.data()));
//     }
//     void API::Init()
//     {
//         // instance
//         {
//             vkb::InstanceBuilder instanceBuilder;
//             auto instance_ret = instanceBuilder
// #ifdef _DEBUG
//                                     .use_default_debug_messenger()
//                                     .request_validation_layers()
// #endif
//                                     .enable_layer("VK_LAYER_KHRONOS_validation")
//                                     .build();
//             if (!instance_ret)
//             {
//                 std::cerr << instance_ret.error().message() << '\n';
//                 return;
//             }
//             init.instance = instance_ret.value();
//         }
//         // surface
//         {
//             GLFWwindow* window = WindowsWindow::Get().GetWindowHandle();
//             init.inst_disp     = init.instance.make_table();
//             VK_CALL(glfwCreateWindowSurface(init.instance, window,
//             nullptr,
//                                             &init.surface));
//         }
//         // device
//         {
//             vkb::PhysicalDeviceSelector
//             phys_device_selector(init.instance); auto phys_device_ret =
//                 phys_device_selector.set_surface(init.surface).select();
//             if (!phys_device_ret)
//             {
//                 std::cerr << phys_device_ret.error().message() << '\n';
//                 return;
//             }
//             vkb::PhysicalDevice physical_device =
//             phys_device_ret.value(); vkb::DeviceBuilder
//             device_builder{physical_device}; auto device_ret =
//             device_builder.build(); if (!device_ret)
//             {
//                 std::cerr << device_ret.error().message() << "\n";
//                 return;
//             }
//             init.device = device_ret.value();
//             init.disp   = init.device.make_table();
//         }
//         // swapchain
//         CreateSwapchain();
//         // queue
//         {
//             auto gq = init.device.get_queue(vkb::QueueType::graphics);
//             if (!gq.has_value())
//             {
//                 std::cerr << "failed to get graphics queue: "
//                           << gq.error().message() << "\n";
//                 return;
//             }
//             apiComps.graphics_queue = gq.value();
//
//             auto pq = init.device.get_queue(vkb::QueueType::present);
//             if (!pq.has_value())
//             {
//                 std::cerr << "failed to get present queue: "
//                           << pq.error().message() << "\n";
//                 return;
//             }
//             apiComps.present_queue = pq.value();
//         }
//         // render pass
//         {
//             VkAttachmentDescription color_attachment = {};
//             color_attachment.format         =
//             init.swapchain.image_format; color_attachment.samples =
//             VK_SAMPLE_COUNT_1_BIT; color_attachment.loadOp         =
//             VK_ATTACHMENT_LOAD_OP_CLEAR; color_attachment.storeOp =
//             VK_ATTACHMENT_STORE_OP_STORE; color_attachment.stencilLoadOp
//             = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//             color_attachment.stencilStoreOp =
//             VK_ATTACHMENT_STORE_OP_DONT_CARE;
//             color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
//             color_attachment.finalLayout    =
//             VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//             VkAttachmentReference color_attachment_ref = {};
//             color_attachment_ref.attachment            = 0;
//             color_attachment_ref.layout =
//                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//             VkSubpassDescription subpass = {};
//             subpass.pipelineBindPoint    =
//             VK_PIPELINE_BIND_POINT_GRAPHICS; subpass.colorAttachmentCount
//             = 1; subpass.pColorAttachments    = &color_attachment_ref;
//
//             VkSubpassDependency dependency = {};
//             dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
//             dependency.dstSubpass          = 0;
//             dependency.srcStageMask =
//                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//             dependency.srcAccessMask = 0;
//             dependency.dstStageMask =
//                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//             dependency.dstAccessMask =
//             VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
//                                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//             VkRenderPassCreateInfo render_pass_info = {};
//             render_pass_info.sType =
//             VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//             render_pass_info.attachmentCount = 1;
//             render_pass_info.pAttachments    = &color_attachment;
//             render_pass_info.subpassCount    = 1;
//             render_pass_info.pSubpasses      = &subpass;
//             render_pass_info.dependencyCount = 1;
//             render_pass_info.pDependencies   = &dependency;
//
//             VK_CALL(init.disp.createRenderPass(&render_pass_info,
//             nullptr,
//                                                &apiComps.render_pass));
//         }
//         {
//             VkDescriptorSetLayoutBinding projectionLayoutBinding{};
//             projectionLayoutBinding.binding         = 0;
//             projectionLayoutBinding.descriptorCount = 1;
//             projectionLayoutBinding.descriptorType =
//                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//             projectionLayoutBinding.stageFlags =
//             VK_SHADER_STAGE_VERTEX_BIT;
//
//             VkDescriptorSetLayoutCreateInfo createInfo{};
//             createInfo.sType =
//                 VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//             createInfo.bindingCount = 1;
//             createInfo.pBindings    = &projectionLayoutBinding;
//             VK_CALL(vkCreateDescriptorSetLayout(init.device, &createInfo,
//                                                 nullptr,
//                                                 &apiComps.descriptorSetLayout));
//         }
//         // graphic pipeline
//         {
//             Shader vert{Renderer2D::GetDevice(), VERT_PATH};
//             Shader frag{Renderer2D::GetDevice(), FRAG_PATH};
//             VkPipelineShaderStageCreateInfo vert_stage_info = {};
//             vert_stage_info.sType =
//                 VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//             vert_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
//             vert_stage_info.module = vert.GetModule();
//             vert_stage_info.pName  = "main";
//
//             VkPipelineShaderStageCreateInfo frag_stage_info = {};
//             frag_stage_info.sType =
//                 VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//             frag_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
//             frag_stage_info.module = frag.GetModule();
//             frag_stage_info.pName  = "main";
//
//             VkPipelineShaderStageCreateInfo shader_stages[] =
//             {vert_stage_info,
//                                                                frag_stage_info};
//
//             auto attributeDescrps = Vertex::GetAttributeDescrp();
//             auto bindingDescr     = Vertex::GetBindingDescription();
//             VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
//             vertexInputInfo.sType =
//                 VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//             vertexInputInfo.pVertexAttributeDescriptions =
//                 attributeDescrps.data();
//             vertexInputInfo.vertexAttributeDescriptionCount =
//                 attributeDescrps.size();
//             vertexInputInfo.pVertexBindingDescriptions    =
//             &bindingDescr; vertexInputInfo.vertexBindingDescriptionCount
//             = 1;
//
//             VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
//             inputAssembly.sType =
//                 VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//             inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//             inputAssembly.primitiveRestartEnable = VK_FALSE;
//
//             VkViewport viewport = {};
//             viewport.x          = 0.0f;
//             viewport.y          = 0.0f;
//             viewport.width      = (float)init.swapchain.extent.width;
//             viewport.height     = (float)init.swapchain.extent.height;
//             viewport.minDepth   = 0.0f;
//             viewport.maxDepth   = 1.0f;
//
//             VkRect2D scissor = {};
//             scissor.offset   = {0, 0};
//             scissor.extent   = init.swapchain.extent;
//
//             VkPipelineViewportStateCreateInfo viewport_state = {};
//             viewport_state.sType =
//                 VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//             viewport_state.viewportCount = 1;
//             viewport_state.pViewports    = &viewport;
//             viewport_state.scissorCount  = 1;
//             viewport_state.pScissors     = &scissor;
//
//             VkPipelineRasterizationStateCreateInfo rasterizer = {};
//             rasterizer.sType =
//                 VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//             rasterizer.depthClampEnable        = VK_FALSE;
//             rasterizer.rasterizerDiscardEnable = VK_FALSE;
//             rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
//             rasterizer.lineWidth               = 1.0f;
//             rasterizer.cullMode                =
//             VK_CULL_MODE_FRONT_AND_BACK; rasterizer.frontFace =
//             VK_FRONT_FACE_CLOCKWISE; rasterizer.depthBiasEnable         =
//             VK_FALSE;
//
//             VkPipelineMultisampleStateCreateInfo multisampling = {};
//             multisampling.sType =
//                 VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//             multisampling.sampleShadingEnable  = VK_FALSE;
//             multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//
//             VkPipelineColorBlendAttachmentState colorBlendAttachment =
//             {}; colorBlendAttachment.colorWriteMask =
//                 VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
//                 VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//             colorBlendAttachment.blendEnable = VK_FALSE;
//
//             VkPipelineColorBlendStateCreateInfo color_blending = {};
//             color_blending.sType =
//                 VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//             color_blending.logicOpEnable     = VK_FALSE;
//             color_blending.logicOp           = VK_LOGIC_OP_COPY;
//             color_blending.attachmentCount   = 1;
//             color_blending.pAttachments      = &colorBlendAttachment;
//             color_blending.blendConstants[0] = 0.0f;
//             color_blending.blendConstants[1] = 0.0f;
//             color_blending.blendConstants[2] = 0.0f;
//             color_blending.blendConstants[3] = 0.0f;
//
//             VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
//                                               VK_DYNAMIC_STATE_SCISSOR};
//
//             VkPipelineDynamicStateCreateInfo dynamic_info = {};
//             dynamic_info.sType =
//                 VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//             dynamic_info.dynamicStateCount = ARRAY_COUNT(dynamicStates);
//             dynamic_info.pDynamicStates    = dynamicStates;
//
//             VkPipelineLayoutCreateInfo pipeline_layout_info = {};
//             pipeline_layout_info.sType =
//                 VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//             pipeline_layout_info.setLayoutCount = 1;
//             pipeline_layout_info.pSetLayouts    =
//             &apiComps.descriptorSetLayout;
//
//             VK_CALL(init.disp.createPipelineLayout(
//                 &pipeline_layout_info, nullptr,
//                 &apiComps.pipeline_layout));
//
//             VkGraphicsPipelineCreateInfo pipeline_info = {};
//             pipeline_info.sType =
//                 VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//             pipeline_info.stageCount          = 2;
//             pipeline_info.pStages             = shader_stages;
//             pipeline_info.pVertexInputState   = &vertexInputInfo;
//             pipeline_info.pInputAssemblyState = &inputAssembly;
//             pipeline_info.pViewportState      = &viewport_state;
//             pipeline_info.pRasterizationState = &rasterizer;
//             pipeline_info.pMultisampleState   = &multisampling;
//             pipeline_info.pColorBlendState    = &color_blending;
//             pipeline_info.pDynamicState       = &dynamic_info;
//             pipeline_info.layout              = apiComps.pipeline_layout;
//             pipeline_info.renderPass          = apiComps.render_pass;
//             pipeline_info.subpass             = 0;
//             pipeline_info.basePipelineHandle  = VK_NULL_HANDLE;
//
//             VK_CALL(init.disp.createGraphicsPipelines(
//                 VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
//                 &apiComps.graphics_pipeline));
//         }
//         CreateFramebuffers();
//         CreateCommandpool();
//         CreateCommandBuffers();
//         {
//             auto size = sizeof(CameraData);
//             // apiComps.uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
//             VkPhysicalDeviceMemoryProperties memProps{};
//             vkGetPhysicalDeviceMemoryProperties(init.device.physical_device,
//                                                 &memProps);
//             for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
//             {
//                 // apiComps.uniformBuffers[i] =
//                 //     Buffer(size, memProps,
//                 //     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//                 //            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
//                 //                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//                 // apiComps.uniformBuffers[i].Init();
//                 // CreateBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//                 //              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
//                 //                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//                 //              apiComps.uniformBuffers[i].GetBuffer(),
//                 //              apiComps.uniformBuffersMems[i]).;
//             }
//         }
//         {
//             VkDescriptorPoolSize poolSize{};
//             poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//             poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;
//             VkDescriptorPoolCreateInfo createInfo{};
//             createInfo.sType =
//             VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//             createInfo.poolSizeCount = 1;
//             createInfo.pPoolSizes    = &poolSize;
//             createInfo.maxSets       = MAX_FRAMES_IN_FLIGHT;
//             VK_CALL(vkCreateDescriptorPool(init.device, &createInfo,
//             nullptr,
//                                            &apiComps.descriptorPool));
//         }
//         {
//             List<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
//                                                 apiComps.descriptorSetLayout);
//             VkDescriptorSetAllocateInfo allocInfo{};
//             allocInfo.sType =
//             VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//             allocInfo.descriptorPool = apiComps.descriptorPool;
//             allocInfo.descriptorSetCount =
//                 static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
//             allocInfo.pSetLayouts = layouts.data();
//
//             apiComps.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
//             VK_CALL(vkAllocateDescriptorSets(init.device, &allocInfo,
//                                              apiComps.descriptorSets.data()));
//             for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
//             {
//                 VkDescriptorBufferInfo bufferInfo{};
//                 bufferInfo.buffer =
//                 *apiComps.uniformBuffers[i].GetBuffer();
//                 bufferInfo.offset = 0;
//                 bufferInfo.range  = sizeof(UniformBufferData);
//
//                 VkWriteDescriptorSet descriptorWrite{};
//                 descriptorWrite.sType  =
//                 VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//                 descriptorWrite.dstSet = apiComps.descriptorSets[i];
//                 descriptorWrite.dstBinding      = 0;
//                 descriptorWrite.dstArrayElement = 0;
//                 descriptorWrite.descriptorType =
//                     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//                 descriptorWrite.descriptorCount = 1;
//                 descriptorWrite.pBufferInfo     = &bufferInfo;
//
//                 vkUpdateDescriptorSets(init.device, 1, &descriptorWrite,
//                 0,
//                                        nullptr);
//             }
//         }
//
//         // SYNC objects
//         {
//             apiComps.available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
//             apiComps.finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
//             apiComps.in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
//             apiComps.image_in_flight.resize(init.swapchain.image_count,
//                                             VK_NULL_HANDLE);
//
//             VkSemaphoreCreateInfo semaphore_info = {};
//             semaphore_info.sType =
//             VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//             VkFenceCreateInfo fence_info = {};
//             fence_info.sType             =
//             VK_STRUCTURE_TYPE_FENCE_CREATE_INFO; fence_info.flags =
//             VK_FENCE_CREATE_SIGNALED_BIT;
//
//             for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
//             {
//                 if (init.disp.createSemaphore(
//                         &semaphore_info, nullptr,
//                         &apiComps.available_semaphores[i]) != VK_SUCCESS
//                         ||
//                     init.disp.createSemaphore(
//                         &semaphore_info, nullptr,
//                         &apiComps.finished_semaphore[i]) != VK_SUCCESS ||
//                     init.disp.createFence(&fence_info, nullptr,
//                                           &apiComps.in_flight_fences[i])
//                                           !=
//                         VK_SUCCESS)
//                 {
//                     std::cout << "failed to create sync objects\n";
//                     return;
//                     // a frame
//                 }
//             }
//         }
//     }
//     void API::UpdateUniformBuffer(UniformBufferData& data)
//     {
//         auto buf = apiComps.uniformBuffers[GetBackBufferIndex()];
//         // buf.SetData(&data, sizeof(UniformBufferData));
//     }
//     void API::WaitForNewImage()
//     {
//         auto currentFrame    = apiComps.current_frame;
//         auto commandBuffer   = apiComps.command_buffers[currentFrame];
//         auto swapchainExtent = init.swapchain.extent;
//         init.disp.waitForFences(1,
//         &apiComps.in_flight_fences[currentFrame],
//                                 VK_TRUE, UINT64_MAX);
//         auto res = init.disp.acquireNextImageKHR(
//             init.swapchain, UINT64_MAX,
//             apiComps.available_semaphores[currentFrame], nullptr,
//             &apiComps.imageIndex);
//         if (res == VK_ERROR_OUT_OF_DATE_KHR)
//         {
//             ResizeSwapchain();
//             return;
//         }
//         init.disp.resetFences(1,
//         &apiComps.in_flight_fences[currentFrame]);
//         vkResetCommandBuffer(apiComps.command_buffers[currentFrame], 0);
//     }
//     void API::StartRecording()
//     {
//         VkCommandBufferBeginInfo beginInfo{};
//         auto currentFrame    = apiComps.current_frame;
//         auto commandBuffer   = apiComps.command_buffers[currentFrame];
//         auto swapchainExtent = init.swapchain.extent;
//
//         beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//
//         if (vkBeginCommandBuffer(commandBuffer, &beginInfo) !=
//         VK_SUCCESS)
//         {
//             throw std::runtime_error(
//                 "failed to begin recording command buffer!");
//         }
//
//         VkRenderPassBeginInfo renderPassInfo{};
//         renderPassInfo.sType       =
//         VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//         renderPassInfo.renderPass  = apiComps.render_pass;
//         renderPassInfo.framebuffer =
//         apiComps.framebuffers[apiComps.imageIndex];
//         renderPassInfo.renderArea.offset = {0, 0};
//         renderPassInfo.renderArea.extent = swapchainExtent;
//
//         renderPassInfo.clearValueCount = 1;
//         renderPassInfo.pClearValues    = &apiComps.clearValue;
//
//         init.disp.cmdBeginRenderPass(commandBuffer, &renderPassInfo,
//                                      VK_SUBPASS_CONTENTS_INLINE);
//         init.disp.cmdBindPipeline(commandBuffer,
//                                   VK_PIPELINE_BIND_POINT_GRAPHICS,
//                                   apiComps.graphics_pipeline);
//
//         VkViewport viewport{};
//         viewport.x        = 0.0f;
//         viewport.y        = 0.0f;
//         viewport.width    = (float)swapchainExtent.width;
//         viewport.height   = (float)swapchainExtent.height;
//         viewport.minDepth = 0.0f;
//         viewport.maxDepth = 1.0f;
//         init.disp.cmdSetViewport(commandBuffer, 0, 1, &viewport);
//
//         VkRect2D scissor{};
//         scissor.offset = {0, 0};
//         scissor.extent = swapchainExtent;
//
//         init.disp.cmdSetScissor(commandBuffer, 0, 1, &scissor);
//         assert(apiComps.vertexBuffer != VK_NULL_HANDLE);
//         VkBuffer vertexBuffers[] = {*apiComps.vertexBuffer};
//         VkDeviceSize offsets[]   = {0};
//         vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers,
//         offsets); vkCmdBindIndexBuffer(commandBuffer,
//         *apiComps.indexBuffer, 0,
//                              VK_INDEX_TYPE_UINT32);
//         vkCmdBindDescriptorSets(commandBuffer,
//         VK_PIPELINE_BIND_POINT_GRAPHICS,
//                                 apiComps.pipeline_layout, 0, 1,
//                                 &apiComps.descriptorSets[currentFrame],
//                                 0, nullptr);
//     }
//     void API::StopRecording()
//     {
//         auto currentFrame  = apiComps.current_frame;
//         auto commandBuffer = apiComps.command_buffers[currentFrame];
//         init.disp.cmdEndRenderPass(commandBuffer);
//         init.disp.endCommandBuffer(commandBuffer);
//     }
//     void API::Submit()
//     {
//         auto currentFrame  = apiComps.current_frame;
//         auto commandBuffer = apiComps.command_buffers[currentFrame];
//         VkSubmitInfo submitInfo{};
//         submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//         VkSemaphore waitSemaphores[] = {
//             apiComps.available_semaphores[currentFrame]};
//         VkPipelineStageFlags waitStages[] = {
//             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
//         submitInfo.waitSemaphoreCount = 1;
//         submitInfo.pWaitSemaphores    = waitSemaphores;
//         submitInfo.pWaitDstStageMask  = waitStages;
//
//         submitInfo.commandBufferCount = 1;
//         submitInfo.pCommandBuffers    = &commandBuffer;
//
//         VkSemaphore signalSemaphores[] = {
//             apiComps.finished_semaphore[currentFrame]};
//         submitInfo.signalSemaphoreCount = 1;
//         submitInfo.pSignalSemaphores    = signalSemaphores;
//         auto res =
//             init.disp.queueSubmit(apiComps.graphics_queue, 1,
//             &submitInfo,
//                                   apiComps.in_flight_fences[currentFrame]);
//
//         if (res != VK_SUCCESS)
//         {
//             std::cerr << "Failed to submit queue" << std::endl;
//         }
//         VkPresentInfoKHR presentInfo{};
//         presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//
//         presentInfo.waitSemaphoreCount = 1;
//         presentInfo.pWaitSemaphores    = signalSemaphores;
//
//         VkSwapchainKHR swapChains[] = {init.swapchain};
//         presentInfo.swapchainCount  = 1;
//         presentInfo.pSwapchains     = swapChains;
//
//         presentInfo.pImageIndices = &apiComps.imageIndex;
//
//         res = init.disp.queuePresentKHR(apiComps.present_queue,
//         &presentInfo);
//         // res = vkQueuePresentKHR(presentQueue, &presentInfo);
//
//         if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
//         {
//             ResizeSwapchain();
//         }
//         else if (res != VK_SUCCESS)
//         {
//             std::cerr << "failed to present swap chain image!" <<
//             std::endl;
//         }
//
//         currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
//     }
//
//     void API::ResizeSwapchain()
//     {
//         init.disp.deviceWaitIdle();
//
//         init.disp.destroyCommandPool(apiComps.command_pool, nullptr);
//
//         for (auto framebuffer : apiComps.framebuffers)
//         {
//             init.disp.destroyFramebuffer(framebuffer, nullptr);
//         }
//         auto ivws = init.swapchain.get_image_views().value();
//         init.swapchain.destroy_image_views(ivws);
//
//         (CreateSwapchain());
//         (CreateFramebuffers());
//         (CreateCommandpool());
//         (CreateCommandBuffers());
//     }
//     static u32 SelectMemoryType(u32 typeFilter,
//                                 VkMemoryPropertyFlags properties)
//     {
//         VkPhysicalDeviceMemoryProperties memProperties;
//         vkGetPhysicalDeviceMemoryProperties(init.device.physical_device,
//                                             &memProperties);
//
//         for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
//         {
//             if ((typeFilter & (1 << i)) &&
//                 (memProperties.memoryTypes[i].propertyFlags & properties)
//                 ==
//                     properties)
//             {
//                 return i;
//             }
//         }
//
//         return ~0u;
//     }
//     void API::DrawIndexed(u32 indexCount, u32 instanceCount, u32
//     firstIndex,
//                           u32 firstInstance, u32 vertexOffset)
//     {
//         auto currentFrame  = apiComps.current_frame;
//         auto commandBuffer = apiComps.command_buffers[currentFrame];
//         vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount,
//         firstIndex,
//                          vertexOffset, firstInstance);
//     }
//     void API::Draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset)
//     {
//         auto currentFrame  = apiComps.current_frame;
//         auto commandBuffer = apiComps.command_buffers[currentFrame];
//         vkCmdDraw(commandBuffer, vertexCount, instanceCount,
//         vertexOffset, 0);
//     }
//     void API::SetVertexBuffer(VkBuffer* buffer)
//     {
//         apiComps.vertexBuffer = buffer;
//     }
//     void API::BindIndexBuffer(VkBuffer* buffer)
//     {
//         apiComps.indexBuffer = buffer;
//     }
//     void API::SetClearColor(VkClearValue clearVal)
//     {
//         apiComps.clearValue = clearVal;
//     }
//     VkDevice API::GetDevice()
//     {
//         return init.device;
//     }
//     VkPhysicalDevice API::GetPhysicalDevice()
//     {
//         return init.device.physical_device;
//     }
//     VkExtent2D API::GetSwapchainExtent()
//     {
//         return init.swapchain.extent;
//     }
// }  // namespace FooGame
