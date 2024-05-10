#include "Api.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <array>
#include "../Backend/VulkanCheckResult.h"
#include "../Core/Base.h"
#include "../Graphics/Shader.h"
#include "../Backend/Vertex.h"
#include "Core/Core/Window.h"
#include "Core/Graphics/Device.h"
#include "vulkan/vulkan_core.h"
namespace FooGame
{

#define VERT_PATH "../../../Shaders/vert.spv"
#define FRAG_PATH "../../../Shaders/frag.spv"
    struct ApiData
    {
            VkInstance instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            Device* device;
            VkSurfaceKHR surface;
            VkRenderPass renderPass;
            VkDescriptorSetLayout descriptorSeLayout;
            GraphicsPipeline graphicsPipeline;
            VkCommandPool commandPool;
            VkDescriptorPool descriptorPool;  // do this on higher level
    };
    ApiData s_Api{};
    static inline VkDevice GetVkDevice()
    {
        return s_Api.device->GetDevice();
    }
    Device* Api::GetDevice()
    {
        return s_Api.device;
    }
    void Api::DestoryBuffer(VkBuffer& buffer)
    {
        vkDestroyBuffer(GetVkDevice(), buffer, nullptr);
    }
    void Api::FreeMemory(VkDeviceMemory& mem)
    {
        vkFreeMemory(GetVkDevice(), mem, nullptr);
    }
    void Api::CreateBuffer(const VkBufferCreateInfo& info, VkBuffer& buffer)
    {
        VK_CALL(vkCreateBuffer(GetVkDevice(), &info, 0, &buffer));
    }
    void Api::AllocateMemory(const VkMemoryAllocateInfo& info,
                             VkDeviceMemory& mem)
    {
        VK_CALL(vkAllocateMemory(GetVkDevice(), &info, 0, &mem));
    }
    void Api::BindBufferMemory(VkBuffer& buffer, VkDeviceMemory& mem,
                               VkDeviceSize deviceSize)
    {
        VK_CALL(vkBindBufferMemory(GetVkDevice(), buffer, mem, deviceSize));
    }
    void Api::UnMapMemory(VkDeviceMemory& mem)
    {
        vkUnmapMemory(GetVkDevice(), mem);
    }

    void Api::CmdCopyBuffer(VkCommandBuffer& cmd, VkBuffer& source,
                            VkBuffer& target, u32 regionCount,
                            VkBufferCopy& region)
    {
        vkCmdCopyBuffer(cmd, source, target, regionCount, &region);
    }
    VkCommandPool Api::GetCommandPool()
    {
        return s_Api.commandPool;
    }
    VkRenderPass Api::GetRenderpass()
    {
        return s_Api.renderPass;
    }
    VkInstance Api::GetInstance()
    {
        return s_Api.instance;
    }

    void Api::WaitIdle()
    {
        vkDeviceWaitIdle(GetVkDevice());
    }
    VkDescriptorPool Api::GetDescriptorPool()
    {
        return s_Api.descriptorPool;
    }
    VkSurfaceKHR Api::GetSurface()
    {
        return s_Api.surface;
    }
    static GraphicsPipeline GetPipeline();
    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }
    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }
    List<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    List<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    Api::~Api()
    {
        Shutdown();
    }
    void Api::Init(WindowsWindow* window)
    {
        {
            VkApplicationInfo appInfo{};
            appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.apiVersion         = VK_API_VERSION_1_0;
            appInfo.pEngineName        = "FooGame Engine";
            appInfo.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
            appInfo.pApplicationName   = "Foo Game";
            appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);

            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo  = &appInfo;
            createInfo.enabledLayerCount = 0;
            createInfo.pNext             = nullptr;

            u32 extensionCount;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
            std::vector<const char*> extensions(
                glfwExtensions, glfwExtensions + extensionCount);
            createInfo.enabledExtensionCount =
                static_cast<u32>(extensions.size());

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
#ifdef _DEBUG

            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            createInfo.enabledLayerCount   = validationLayers.size();
            createInfo.ppEnabledLayerNames = validationLayers.data();

            debugCreateInfo.sType =
                VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = debugCallback;

            createInfo.pNext =
                (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif
            createInfo.ppEnabledExtensionNames = extensions.data();
            createInfo.enabledExtensionCount =
                static_cast<u32>(extensions.size());
            VK_CALL(vkCreateInstance(&createInfo, nullptr, &s_Api.instance));

#ifdef _DEBUG
            VK_CALL(CreateDebugUtilsMessengerEXT(s_Api.instance,
                                                 &debugCreateInfo, nullptr,
                                                 &s_Api.debugMessenger));
#endif
            VK_CALL(glfwCreateWindowSurface(s_Api.instance,
                                            window->GetWindowHandle(), nullptr,
                                            &s_Api.surface));
        }
        {
            s_Api.device =
                Device::CreateDevice(deviceExtensions, validationLayers);
        }
    }

    void Api::CreateCommandPool()
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = s_Api.device->GetGraphicsFamily();

        VK_CALL(vkCreateCommandPool(s_Api.device->GetDevice(), &poolInfo,
                                    nullptr, &s_Api.commandPool));
    }

    void Api::CreateDescriptorPool()
    {
        VkDescriptorPoolSize poolSizes[2] = {};
        poolSizes[0].type                 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount      = MAX_FRAMES_IN_FLIGHT;

        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = ARRAY_COUNT(poolSizes);
        poolInfo.pPoolSizes    = poolSizes;
        poolInfo.maxSets       = MAX_FRAMES_IN_FLIGHT;
        VK_CALL(vkCreateDescriptorPool(s_Api.device->GetDevice(), &poolInfo,
                                       nullptr, &s_Api.descriptorPool));
    }
    void Api::CreateRenderpass(VkFormat colorAttachmentFormat)
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format         = colorAttachmentFormat;
        colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format         = VK_FORMAT_D32_SFLOAT;
        depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout =
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout =
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment,
                                                              depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount =
            static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        VK_CALL(vkCreateRenderPass(s_Api.device->GetDevice(), &renderPassInfo,
                                   nullptr, &s_Api.renderPass));
    }

    void Api::SetupDesciptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding            = 0;
        uboLayoutBinding.descriptorCount    = 1;
        uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding         = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding,
                                                    samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = ARRAY_COUNT(bindings);
        layoutInfo.pBindings    = bindings;

        VK_CALL(vkCreateDescriptorSetLayout(s_Api.device->GetDevice(),
                                            &layoutInfo, nullptr,
                                            &s_Api.descriptorSeLayout));
    }

    void Api::CreateGraphicsPipeline()
    {
        Shader vert{s_Api.device->GetDevice(), VERT_PATH};
        Shader frag{s_Api.device->GetDevice(), FRAG_PATH};
        auto vertShaderStageInfo = vert.CreateInfo(VK_SHADER_STAGE_VERTEX_BIT);

        auto fragShaderStageInfo =
            frag.CreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT);

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                          fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription    = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescrp();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions =
            attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount  = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable        = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth               = 1.0f;
        rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable         = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable  = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable       = VK_TRUE;
        depthStencil.depthWriteEnable      = VK_TRUE;
        depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable     = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable     = VK_FALSE;
        colorBlending.logicOp           = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount   = 1;
        colorBlending.pAttachments      = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                          VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = ARRAY_COUNT(dynamicStates);
        dynamicState.pDynamicStates    = dynamicStates;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts    = &s_Api.descriptorSeLayout;

        VK_CALL(vkCreatePipelineLayout(s_Api.device->GetDevice(),
                                       &pipelineLayoutInfo, nullptr,
                                       &s_Api.graphicsPipeline.pipelineLayout));
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = shaderStages;
        pipelineInfo.pVertexInputState   = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState      = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState   = &multisampling;
        pipelineInfo.pDepthStencilState  = &depthStencil;
        pipelineInfo.pColorBlendState    = &colorBlending;
        pipelineInfo.pDynamicState       = &dynamicState;
        pipelineInfo.layout             = s_Api.graphicsPipeline.pipelineLayout;
        pipelineInfo.renderPass         = s_Api.renderPass;
        pipelineInfo.subpass            = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        VK_CALL(vkCreateGraphicsPipelines(
            s_Api.device->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo,
            nullptr, &s_Api.graphicsPipeline.pipeline));
    }
    GraphicsPipeline Api::GetPipeline()
    {
        return s_Api.graphicsPipeline;
    }
    void Api::SetViewportAndScissors(VkCommandBuffer cmd, float w, float h)
    {
        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = w;
        viewport.height   = h;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        VkRect2D scissor{
            {                               0,0                                            },
            {static_cast<u32>(viewport.width),
             static_cast<u32>(viewport.height)}
        };
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void Api::Shutdown()
    {
        auto device = GetVkDevice();
        vkDestroyPipeline(device, s_Api.graphicsPipeline.pipeline, nullptr);
        vkDestroyPipelineLayout(device, s_Api.graphicsPipeline.pipelineLayout,
                                nullptr);
        vkDestroyRenderPass(device, s_Api.renderPass, nullptr);
        vkDestroyDescriptorPool(device, s_Api.descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device, s_Api.descriptorSeLayout, nullptr);
        vkDestroyCommandPool(device, s_Api.commandPool, nullptr);
        vkDestroyDevice(device, nullptr);

#ifdef _DEBUG
        DestroyDebugUtilsMessengerEXT(s_Api.instance, s_Api.debugMessenger,
                                      nullptr);
#endif
        vkDestroySurfaceKHR(s_Api.instance, s_Api.surface, nullptr);
        vkDestroyInstance(s_Api.instance, nullptr);
    }

}  // namespace FooGame
