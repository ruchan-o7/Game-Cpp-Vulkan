#pragma once
#include "Misc.h"
#include "VulkanHeader.h"

namespace fg {

VkDescriptorType ToVk(DescriptorType type);
VkFormat ToVk(fg::ValueType type);

VkVertexInputRate ToVk(fg::VertexInputRate rate);
VkFilter ToVk(fg::Filter filter);

VkSamplerAddressMode ToVk(fg::SamplerAddressMode mode);

VkBorderColor ToVk(fg::SamplerBorderColor color);
VkCompareOp ToVk(fg::CompareOperation op);

ResourceState VkImageLayoutToResouceState(VkImageLayout layout);
bool ResourceStateHasWriteAccess(ResourceState oldState);
VkImageLayout ResourceStateToVkImageLayout(ResourceState state);
VkPipelineStageFlags ResourceStateFlagsToVkPipelineStageFlags(ResourceState state);

}  // namespace fg
