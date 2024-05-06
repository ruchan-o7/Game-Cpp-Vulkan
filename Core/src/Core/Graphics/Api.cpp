#include "Api.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "../Backend/VulkanCheckResult.h"
#include "../Core/Base.h"
#include "../Graphics/Shader.h"
#include "../Backend/Vertex.h"
#include "Core/Graphics/Swapchain.h"
namespace FooGame
{
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
#define VERT_PATH \
    "C:\\Users\\jcead\\dev\\CppProjects\\game_1\\Shaders\\vert.spv"
#define FRAG_PATH \
    "C:\\Users\\jcead\\dev\\CppProjects\\game_1\\Shaders\\frag.spv"
    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
    Api::~Api()
    {
        Shutdown();
    }
    void Api::Init(GLFWwindow* window)
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
            createInfo.enabledLayerCount   = ARRAY_COUNT(validationLayers);
            createInfo.ppEnabledLayerNames = validationLayers;

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
            VK_CALL(vkCreateInstance(&createInfo, nullptr, &m_Instance));

#ifdef _DEBUG
            VK_CALL(CreateDebugUtilsMessengerEXT(m_Instance, &debugCreateInfo,
                                                 nullptr, &debugMessenger));
#endif
            VK_CALL(glfwCreateWindowSurface(m_Instance, window, nullptr,
                                            &m_Surface));
        }
        {
            DeviceCreateBuilder deviceBuilder{m_Instance};
            deviceBuilder.AddLayer(*validationLayers);
            deviceBuilder.AddExtension(*deviceExtensions);
            m_Device = deviceBuilder.Build();
        }
    }

    void Api::CreateCommandPool()
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_Device->GetGraphicsFamily();

        VK_CALL(vkCreateCommandPool(m_Device->GetDevice(), &poolInfo, nullptr,
                                    &m_CommandPool));
    }

    void Api::CreateDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes    = &poolSize;
        poolInfo.maxSets       = MAX_FRAMES_IN_FLIGHT;

        VK_CALL(vkCreateDescriptorPool(m_Device->GetDevice(), &poolInfo,
                                       nullptr, &m_DescriptorPool));
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

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments    = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass   = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass   = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments    = &colorAttachment;
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        VK_CALL(vkCreateRenderPass(m_Device->GetDevice(), &renderPassInfo,
                                   nullptr, &m_RenderPass));
    }

    void Api::SetupDesciptorLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding            = 0;
        uboLayoutBinding.descriptorCount    = 1;
        uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings    = &uboLayoutBinding;
        VK_CALL(vkCreateDescriptorSetLayout(m_Device->GetDevice(), &layoutInfo,
                                            nullptr, &m_DescriptorSetLayout));
    }

    void Api::CreateGraphicsPipeline()
    {
        Shader vert{m_Device->GetDevice(), VERT_PATH};
        Shader frag{m_Device->GetDevice(), FRAG_PATH};
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

        std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                                     VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount =
            static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts    = &m_DescriptorSetLayout;

        VK_CALL(vkCreatePipelineLayout(m_Device->GetDevice(),
                                       &pipelineLayoutInfo, nullptr,
                                       &m_GraphicsPipeline.pipelineLayout));
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = shaderStages;
        pipelineInfo.pVertexInputState   = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState      = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState   = &multisampling;
        pipelineInfo.pColorBlendState    = &colorBlending;
        pipelineInfo.pDynamicState       = &dynamicState;
        pipelineInfo.layout              = m_GraphicsPipeline.pipelineLayout;
        pipelineInfo.renderPass          = m_RenderPass;
        pipelineInfo.subpass             = 0;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

        VK_CALL(vkCreateGraphicsPipelines(m_Device->GetDevice(), VK_NULL_HANDLE,
                                          1, &pipelineInfo, nullptr,
                                          &m_GraphicsPipeline.pipeline));
    }

    void Api::Shutdown()
    {
        auto device = m_Device->GetDevice();
        vkDestroyPipeline(device, m_GraphicsPipeline.pipeline, nullptr);
        vkDestroyPipelineLayout(device, m_GraphicsPipeline.pipelineLayout,
                                nullptr);
        vkDestroyRenderPass(device, m_RenderPass, nullptr);
        vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
        vkDestroyCommandPool(device, m_CommandPool, nullptr);
        vkDestroyDevice(device, nullptr);

#ifdef _DEBUG
        DestroyDebugUtilsMessengerEXT(m_Instance, debugMessenger, nullptr);
#endif
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyInstance(m_Instance, nullptr);
    }

}  // namespace FooGame
