#pragma once
#include <vulkan/vulkan.h>
namespace Engine
{

    struct DescriptorData
    {
            VkDescriptorPool pool;
            VkDescriptorSetLayout SetLayout;
            // VkDescriptorSet Sets[3];
    };

}  // namespace Engine
