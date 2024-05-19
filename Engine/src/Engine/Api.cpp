#include "Api.h"
#include <array>
#include <GLFW/glfw3.h>
#include "Device.h"
#include "VulkanCheckResult.h"
#include "../Window/Window.h"
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
            VkCommandPool commandPool;
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
                            VkBuffer& target, uint32_t regionCount,
                            VkBufferCopy& region)
    {
        vkCmdCopyBuffer(cmd, source, target, regionCount, &region);
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
    VkSurfaceKHR Api::GetSurface()
    {
        return s_Api.surface;
    }
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
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
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

            uint32_t extensionCount;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
            std::vector<const char*> extensions(
                glfwExtensions, glfwExtensions + extensionCount);
            createInfo.enabledExtensionCount =
                static_cast<uint32_t>(extensions.size());

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
#ifdef FOO_DEBUG

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
                static_cast<uint32_t>(extensions.size());
            VK_CALL(vkCreateInstance(&createInfo, nullptr, &s_Api.instance));

#ifdef FOO_DEBUG
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
    void Api::SetViewportAndScissors(VkCommandBuffer cmd, float w, float h)
    {
        VkViewport viewport{};
        viewport.x        = 0;
        viewport.y        = 0;
        viewport.width    = w;
        viewport.height   = h;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        VkRect2D scissor{
            {                                    0,0                                                 },
            {static_cast<uint32_t>(viewport.width),
             static_cast<uint32_t>(viewport.height)}
        };
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void Api::Shutdown()
    {
        auto device = GetVkDevice();
        vkDestroyRenderPass(device, s_Api.renderPass, nullptr);
        vkDestroyCommandPool(device, s_Api.commandPool, nullptr);
        vkDestroyDevice(device, nullptr);

#ifdef FOO_DEBUG
        DestroyDebugUtilsMessengerEXT(s_Api.instance, s_Api.debugMessenger,
                                      nullptr);
#endif
        vkDestroySurfaceKHR(s_Api.instance, s_Api.surface, nullptr);
        vkDestroyInstance(s_Api.instance, nullptr);
    }

}  // namespace FooGame
