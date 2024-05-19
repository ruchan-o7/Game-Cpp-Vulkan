#pragma once
#include <vulkan/vulkan.h>
namespace FooGame
{

    struct DescriptorData
    {
            VkDescriptorPool pool;
            VkDescriptorSetLayout SetLayout;
            // VkDescriptorSet Sets[3];
    };

}  // namespace FooGame
