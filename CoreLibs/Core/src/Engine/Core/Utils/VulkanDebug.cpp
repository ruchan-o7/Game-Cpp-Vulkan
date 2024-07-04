#include "VulkanDebug.h"
#include <Log.h>
#include <ios>
#include <iomanip>
#include <sstream>
#include "src/Engine/Core/VulkanLogicalDevice.h"

namespace ENGINE_NAMESPACE
{

    PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT   = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT = nullptr;
    PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT       = nullptr;
    PFN_vkSetDebugUtilsObjectTagEXT SetDebugUtilsObjectTagEXT         = nullptr;
    PFN_vkQueueBeginDebugUtilsLabelEXT QueueBeginDebugUtilsLabelEXT   = nullptr;
    PFN_vkQueueEndDebugUtilsLabelEXT QueueEndDebugUtilsLabelEXT       = nullptr;
    PFN_vkQueueInsertDebugUtilsLabelEXT QueueInsertDebugUtilsLabelEXT = nullptr;

    VkDebugUtilsMessengerEXT DbgMessenger = VK_NULL_HANDLE;

    VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                           VkDebugUtilsMessageTypeFlagsEXT messageType,
                           const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
    {
        std::stringstream debugMessage;
        debugMessage << "Vulkan debug message (";
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        {
            debugMessage << "general";
            if (messageType & (VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
            {
                debugMessage << ", ";
            }
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        {
            debugMessage << "validation";
            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            {
                debugMessage << ", ";
            }
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        {
            debugMessage << "performance";
        }
        debugMessage << "): ";

        debugMessage << (callbackData->pMessageIdName != nullptr ? callbackData->pMessageIdName
                                                                 : "<Unknown name>");
        if (callbackData->pMessage != nullptr)
        {
            debugMessage << std::endl << "                 " << callbackData->pMessage;
        }

        if (callbackData->objectCount > 0)
        {
            for (uint32_t obj = 0; obj < callbackData->objectCount; ++obj)
            {
                const auto& Object = callbackData->pObjects[obj];
                debugMessage << std::endl
                             << "                 Object[" << obj << "] ("
                             << VkObjectTypeToString(Object.objectType) << "): Handle " << std::hex
                             << "0x" << Object.objectHandle;
                if (Object.pObjectName != nullptr)
                {
                    debugMessage << ", Name: '" << Object.pObjectName << '\'';
                }
            }
        }

        if (callbackData->cmdBufLabelCount > 0)
        {
            for (uint32_t l = 0; l < callbackData->cmdBufLabelCount; ++l)
            {
                const auto& Label = callbackData->pCmdBufLabels[l];
                debugMessage << std::endl << "                 Label[" << l << "]";
                if (Label.pLabelName != nullptr)
                {
                    debugMessage << " - " << Label.pLabelName;
                }
                debugMessage << " {";
                debugMessage << std::fixed << std::setw(4) << Label.color[0] << ", " << std::fixed
                             << std::setw(4) << Label.color[1] << ", " << std::fixed << std::setw(4)
                             << Label.color[2] << ", " << std::fixed << std::setw(4)
                             << Label.color[3] << "}";
            }
        }

        FOO_ENGINE_WARN(debugMessage.str().c_str());
        return VK_FALSE;
    }

    bool SetupDebugUtils(VkInstance instance, VkDebugUtilsMessageSeverityFlagsEXT messageSeverity,
                         VkDebugUtilsMessageTypeFlagsEXT messageType, uint32_t IgnoreMessageCount,
                         const char* const* ppIgnoreMessageNames, void* pUserData)
    {
        CreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        DestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (CreateDebugUtilsMessengerEXT == nullptr || DestroyDebugUtilsMessengerEXT == nullptr)
        {
            return false;
        }

        VkDebugUtilsMessengerCreateInfoEXT DbgMessenger_CI{};
        DbgMessenger_CI.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        DbgMessenger_CI.pNext           = NULL;
        DbgMessenger_CI.flags           = 0;
        DbgMessenger_CI.messageSeverity = messageSeverity;
        DbgMessenger_CI.messageType     = messageType;
        DbgMessenger_CI.pfnUserCallback = DebugMessengerCallback;
        DbgMessenger_CI.pUserData       = pUserData;

        auto err = CreateDebugUtilsMessengerEXT(instance, &DbgMessenger_CI, nullptr, &DbgMessenger);
        assert(err == VK_SUCCESS && "Failed to create debug utils messenger");

        // Load function pointers
        SetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
            vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT"));
        assert(SetDebugUtilsObjectNameEXT != nullptr);
        SetDebugUtilsObjectTagEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectTagEXT>(
            vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectTagEXT"));
        assert(SetDebugUtilsObjectTagEXT != nullptr);

        QueueBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkQueueBeginDebugUtilsLabelEXT>(
            vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT"));
        assert(QueueBeginDebugUtilsLabelEXT != nullptr);
        QueueEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkQueueEndDebugUtilsLabelEXT>(
            vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT"));
        assert(QueueEndDebugUtilsLabelEXT != nullptr);
        QueueInsertDebugUtilsLabelEXT = reinterpret_cast<PFN_vkQueueInsertDebugUtilsLabelEXT>(
            vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT"));
        assert(QueueInsertDebugUtilsLabelEXT != nullptr);

        return err == VK_SUCCESS;
    }

    void FreeDebug(VkInstance instance)
    {
        if (DbgMessenger != VK_NULL_HANDLE)
        {
            DestroyDebugUtilsMessengerEXT(instance, DbgMessenger, nullptr);
        }
    }

    void InsertCmdQueueLabel(VkQueue cmdQueue, const char* pLabelName, const float* color)
    {
        if (QueueInsertDebugUtilsLabelEXT == nullptr)
        {
            return;
        }

        VkDebugUtilsLabelEXT Label{};
        Label.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        Label.pNext      = nullptr;
        Label.pLabelName = pLabelName;
        for (int i = 0; i < 4; ++i)
        {
            Label.color[i] = color[i];
        }
        QueueInsertDebugUtilsLabelEXT(cmdQueue, &Label);
    }

    void EndCmdQueueLabelRegion(VkQueue cmdQueue)
    {
        if (QueueEndDebugUtilsLabelEXT == nullptr)
        {
            return;
        }

        QueueEndDebugUtilsLabelEXT(cmdQueue);
    }

    void SetObjectName(VkDevice device, uint64_t object, VkObjectType objectType, const char* name)
    {
        // Check for valid function pointer (may not be present if not running in a debug mode)
        if (SetDebugUtilsObjectNameEXT == nullptr || name == nullptr || name[0] == '\0')
        {
            return;
        }

        VkDebugUtilsObjectNameInfoEXT ObjectNameInfo{};
        ObjectNameInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        ObjectNameInfo.pNext        = nullptr;
        ObjectNameInfo.objectType   = objectType;
        ObjectNameInfo.objectHandle = object;
        ObjectNameInfo.pObjectName  = name;

        VkResult res = SetDebugUtilsObjectNameEXT(device, &ObjectNameInfo);
        assert(res == VK_SUCCESS);
        (void)res;
    }

    void SetObjectTag(VkDevice device, uint64_t objectHandle, VkObjectType objectType,
                      uint64_t name, size_t tagSize, const void* tag)
    {
        // Check for valid function pointer (may not be present if not running in a debugging
        // application)
        if (SetDebugUtilsObjectTagEXT == nullptr)
        {
            return;
        }

        VkDebugUtilsObjectTagInfoEXT tagInfo{};
        tagInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT;
        tagInfo.pNext        = nullptr;
        tagInfo.objectType   = objectType;
        tagInfo.objectHandle = objectHandle;
        tagInfo.tagName      = name;
        tagInfo.tagSize      = tagSize;
        tagInfo.pTag         = tag;
        SetDebugUtilsObjectTagEXT(device, &tagInfo);
    }

    void SetCommandPoolName(VkDevice device, VkCommandPool cmdPool, const char* name)
    {
        SetObjectName(device, (uint64_t)cmdPool, VK_OBJECT_TYPE_COMMAND_POOL, name);
    }

    void SetCommandBufferName(VkDevice device, VkCommandBuffer cmdBuffer, const char* name)
    {
        SetObjectName(device, (uint64_t)cmdBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, name);
    }

    void SetQueueName(VkDevice device, VkQueue queue, const char* name)
    {
        SetObjectName(device, (uint64_t)queue, VK_OBJECT_TYPE_QUEUE, name);
    }

    void SetImageName(VkDevice device, VkImage image, const char* name)
    {
        SetObjectName(device, (uint64_t)image, VK_OBJECT_TYPE_IMAGE, name);
    }

    void SetImageViewName(VkDevice device, VkImageView imageView, const char* name)
    {
        SetObjectName(device, (uint64_t)imageView, VK_OBJECT_TYPE_IMAGE_VIEW, name);
    }

    void SetSamplerName(VkDevice device, VkSampler sampler, const char* name)
    {
        SetObjectName(device, (uint64_t)sampler, VK_OBJECT_TYPE_SAMPLER, name);
    }

    void SetBufferName(VkDevice device, VkBuffer buffer, const char* name)
    {
        SetObjectName(device, (uint64_t)buffer, VK_OBJECT_TYPE_BUFFER, name);
    }

    void SetBufferViewName(VkDevice device, VkBufferView bufferView, const char* name)
    {
        SetObjectName(device, (uint64_t)bufferView, VK_OBJECT_TYPE_BUFFER_VIEW, name);
    }

    void SetDeviceMemoryName(VkDevice device, VkDeviceMemory memory, const char* name)
    {
        SetObjectName(device, (uint64_t)memory, VK_OBJECT_TYPE_DEVICE_MEMORY, name);
    }

    void SetShaderModuleName(VkDevice device, VkShaderModule shaderModule, const char* name)
    {
        SetObjectName(device, (uint64_t)shaderModule, VK_OBJECT_TYPE_SHADER_MODULE, name);
    }

    void SetPipelineName(VkDevice device, VkPipeline pipeline, const char* name)
    {
        SetObjectName(device, (uint64_t)pipeline, VK_OBJECT_TYPE_PIPELINE, name);
    }

    void SetPipelineLayoutName(VkDevice device, VkPipelineLayout pipelineLayout, const char* name)
    {
        SetObjectName(device, (uint64_t)pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, name);
    }

    void SetRenderPassName(VkDevice device, VkRenderPass renderPass, const char* name)
    {
        SetObjectName(device, (uint64_t)renderPass, VK_OBJECT_TYPE_RENDER_PASS, name);
    }

    void SetFramebufferName(VkDevice device, VkFramebuffer framebuffer, const char* name)
    {
        SetObjectName(device, (uint64_t)framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, name);
    }

    void SetDescriptorSetLayoutName(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                    const char* name)
    {
        SetObjectName(device, (uint64_t)descriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                      name);
    }

    void SetDescriptorSetName(VkDevice device, VkDescriptorSet descriptorSet, const char* name)
    {
        SetObjectName(device, (uint64_t)descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, name);
    }

    void SetDescriptorPoolName(VkDevice device, VkDescriptorPool descriptorPool, const char* name)
    {
        SetObjectName(device, (uint64_t)descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, name);
    }

    void SetSemaphoreName(VkDevice device, VkSemaphore semaphore, const char* name)
    {
        SetObjectName(device, (uint64_t)semaphore, VK_OBJECT_TYPE_SEMAPHORE, name);
    }

    void SetFenceName(VkDevice device, VkFence fence, const char* name)
    {
        SetObjectName(device, (uint64_t)fence, VK_OBJECT_TYPE_FENCE, name);
    }

    void SetEventName(VkDevice device, VkEvent _event, const char* name)
    {
        SetObjectName(device, (uint64_t)_event, VK_OBJECT_TYPE_EVENT, name);
    }

    void SetQueryPoolName(VkDevice device, VkQueryPool queryPool, const char* name)
    {
        SetObjectName(device, (uint64_t)queryPool, VK_OBJECT_TYPE_QUERY_POOL, name);
    }

    void SetAccelStructName(VkDevice device, VkAccelerationStructureKHR accelStruct,
                            const char* name)
    {
        SetObjectName(device, (uint64_t)accelStruct, VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR,
                      name);
    }

    void SetPipelineCacheName(VkDevice device, VkPipelineCache pipeCache, const char* name)
    {
        SetObjectName(device, (uint64_t)pipeCache, VK_OBJECT_TYPE_PIPELINE_CACHE, name);
    }

    template <>
    void SetVulkanObjectName<VkCommandPool, VulkanHandleTypeId::CommandPool>(VkDevice device,
                                                                             VkCommandPool cmdPool,
                                                                             const char* name)
    {
        SetCommandPoolName(device, cmdPool, name);
    }

    template <>
    void SetVulkanObjectName<VkCommandBuffer, VulkanHandleTypeId::CommandBuffer>(
        VkDevice device, VkCommandBuffer cmdBuffer, const char* name)
    {
        SetCommandBufferName(device, cmdBuffer, name);
    }

    template <>
    void SetVulkanObjectName<VkQueue, VulkanHandleTypeId::Queue>(VkDevice device, VkQueue queue,
                                                                 const char* name)
    {
        SetQueueName(device, queue, name);
    }

    template <>
    void SetVulkanObjectName<VkImage, VulkanHandleTypeId::Image>(VkDevice device, VkImage image,
                                                                 const char* name)
    {
        SetImageName(device, image, name);
    }

    template <>
    void SetVulkanObjectName<VkImageView, VulkanHandleTypeId::ImageView>(VkDevice device,
                                                                         VkImageView imageView,
                                                                         const char* name)
    {
        SetImageViewName(device, imageView, name);
    }

    template <>
    void SetVulkanObjectName<VkSampler, VulkanHandleTypeId::Sampler>(VkDevice device,
                                                                     VkSampler sampler,
                                                                     const char* name)
    {
        SetSamplerName(device, sampler, name);
    }

    template <>
    void SetVulkanObjectName<VkBuffer, VulkanHandleTypeId::Buffer>(VkDevice device, VkBuffer buffer,
                                                                   const char* name)
    {
        SetBufferName(device, buffer, name);
    }

    template <>
    void SetVulkanObjectName<VkBufferView, VulkanHandleTypeId::BufferView>(VkDevice device,
                                                                           VkBufferView bufferView,
                                                                           const char* name)
    {
        SetBufferViewName(device, bufferView, name);
    }

    template <>
    void SetVulkanObjectName<VkDeviceMemory, VulkanHandleTypeId::DeviceMemory>(
        VkDevice device, VkDeviceMemory memory, const char* name)
    {
        SetDeviceMemoryName(device, memory, name);
    }

    template <>
    void SetVulkanObjectName<VkShaderModule, VulkanHandleTypeId::ShaderModule>(
        VkDevice device, VkShaderModule shaderModule, const char* name)
    {
        SetShaderModuleName(device, shaderModule, name);
    }

    template <>
    void SetVulkanObjectName<VkPipeline, VulkanHandleTypeId::Pipeline>(VkDevice device,
                                                                       VkPipeline pipeline,
                                                                       const char* name)
    {
        SetPipelineName(device, pipeline, name);
    }

    template <>
    void SetVulkanObjectName<VkPipelineLayout, VulkanHandleTypeId::PipelineLayout>(
        VkDevice device, VkPipelineLayout pipelineLayout, const char* name)
    {
        SetPipelineLayoutName(device, pipelineLayout, name);
    }

    template <>
    void SetVulkanObjectName<VkRenderPass, VulkanHandleTypeId::RenderPass>(VkDevice device,
                                                                           VkRenderPass renderPass,
                                                                           const char* name)
    {
        SetRenderPassName(device, renderPass, name);
    }

    template <>
    void SetVulkanObjectName<VkFramebuffer, VulkanHandleTypeId::Framebuffer>(
        VkDevice device, VkFramebuffer framebuffer, const char* name)
    {
        SetFramebufferName(device, framebuffer, name);
    }

    template <>
    void SetVulkanObjectName<VkDescriptorSetLayout, VulkanHandleTypeId::DescriptorSetLayout>(
        VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const char* name)
    {
        SetDescriptorSetLayoutName(device, descriptorSetLayout, name);
    }

    template <>
    void SetVulkanObjectName<VkDescriptorSet, VulkanHandleTypeId::DescriptorSet>(
        VkDevice device, VkDescriptorSet descriptorSet, const char* name)
    {
        SetDescriptorSetName(device, descriptorSet, name);
    }

    template <>
    void SetVulkanObjectName<VkDescriptorPool, VulkanHandleTypeId::DescriptorPool>(
        VkDevice device, VkDescriptorPool descriptorPool, const char* name)
    {
        SetDescriptorPoolName(device, descriptorPool, name);
    }

    template <>
    void SetVulkanObjectName<VkSemaphore, VulkanHandleTypeId::Semaphore>(VkDevice device,
                                                                         VkSemaphore semaphore,
                                                                         const char* name)
    {
        SetSemaphoreName(device, semaphore, name);
    }

    template <>
    void SetVulkanObjectName<VkFence, VulkanHandleTypeId::Fence>(VkDevice device, VkFence fence,
                                                                 const char* name)
    {
        SetFenceName(device, fence, name);
    }

    template <>
    void SetVulkanObjectName<VkEvent, VulkanHandleTypeId::Event>(VkDevice device, VkEvent _event,
                                                                 const char* name)
    {
        SetEventName(device, _event, name);
    }

    template <>
    void SetVulkanObjectName<VkQueryPool, VulkanHandleTypeId::QueryPool>(VkDevice device,
                                                                         VkQueryPool queryPool,
                                                                         const char* name)
    {
        SetQueryPoolName(device, queryPool, name);
    }

    template <>
    void
    SetVulkanObjectName<VkAccelerationStructureKHR, VulkanHandleTypeId::AccelerationStructureKHR>(
        VkDevice device, VkAccelerationStructureKHR accelStruct, const char* name)
    {
        SetAccelStructName(device, accelStruct, name);
    }

    template <>
    void SetVulkanObjectName<VkPipelineCache, VulkanHandleTypeId::PipelineCache>(
        VkDevice device, VkPipelineCache pipeCache, const char* name)
    {
        SetPipelineCacheName(device, pipeCache, name);
    }
    const char* VkObjectTypeToString(VkObjectType ObjectType)
    {
        switch (ObjectType)
        {
                // clang-format off
        case VK_OBJECT_TYPE_UNKNOWN:                        return "unknown";
        case VK_OBJECT_TYPE_INSTANCE:                       return "instance";
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE:                return "physical device";
        case VK_OBJECT_TYPE_DEVICE:                         return "device";
        case VK_OBJECT_TYPE_QUEUE:                          return "queue";
        case VK_OBJECT_TYPE_SEMAPHORE:                      return "semaphore";
        case VK_OBJECT_TYPE_COMMAND_BUFFER:                 return "cmd buffer";
        case VK_OBJECT_TYPE_FENCE:                          return "fence";
        case VK_OBJECT_TYPE_DEVICE_MEMORY:                  return "memory";
        case VK_OBJECT_TYPE_BUFFER:                         return "buffer";
        case VK_OBJECT_TYPE_IMAGE:                          return "image";
        case VK_OBJECT_TYPE_EVENT:                          return "event";
        case VK_OBJECT_TYPE_QUERY_POOL:                     return "query pool";
        case VK_OBJECT_TYPE_BUFFER_VIEW:                    return "buffer view";
        case VK_OBJECT_TYPE_IMAGE_VIEW:                     return "image view";
        case VK_OBJECT_TYPE_SHADER_MODULE:                  return "shader module";
        case VK_OBJECT_TYPE_PIPELINE_CACHE:                 return "pipeline cache";
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT:                return "pipeline layout";
        case VK_OBJECT_TYPE_RENDER_PASS:                    return "render pass";
        case VK_OBJECT_TYPE_PIPELINE:                       return "pipeline";
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:          return "descriptor set layout";
        case VK_OBJECT_TYPE_SAMPLER:                        return "sampler";
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL:                return "descriptor pool";
        case VK_OBJECT_TYPE_DESCRIPTOR_SET:                 return "descriptor set";
        case VK_OBJECT_TYPE_FRAMEBUFFER:                    return "framebuffer";
        case VK_OBJECT_TYPE_COMMAND_POOL:                   return "command pool";
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:       return "sampler ycbcr conversion";
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:     return "descriptor update template";
        case VK_OBJECT_TYPE_SURFACE_KHR:                    return "surface KHR";
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR:                  return "swapchain KHR";
        case VK_OBJECT_TYPE_DISPLAY_KHR:                    return "display KHR";
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:               return "display mode KHR";
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:      return "debug report callback";
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:      return "debug utils messenger";
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR:     return "acceleration structure KHR";
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:           return "validation cache EXT";
        case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL:return "performance configuration INTEL";
        case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR:         return "deferred operation KHR";
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV:    return "indirect commands layout NV";
        case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT:          return "private data slot EXT";
        default: return "unknown";
                // clang-format on
        }
    }
}  // namespace ENGINE_NAMESPACE
