#include "TypeConversions.h"
#include "../Core/Assert.h"
#include "Misc.h"

namespace fg {

VkDescriptorType ToVk(DescriptorType type) {
  switch (type) {
    case DescriptorType::Sampler:
      return VK_DESCRIPTOR_TYPE_SAMPLER;
    case DescriptorType::CombinedImageSampler:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case DescriptorType::SampledImage:
      return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case DescriptorType::UniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case DescriptorType::UniformTexelBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case DescriptorType::UniformBufferDynamic:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case DescriptorType::StorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case DescriptorType::InputAttachment:
      return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
  }
}

VkFormat ToVk(fg::ValueType type) {
  switch (type) {
    case fg::VT_FLOAT:
      return VK_FORMAT_R32_SFLOAT;
    case fg::VT_VEC2:
      return VK_FORMAT_R32G32_SFLOAT;
    case fg::VT_VEC3:
      return VK_FORMAT_R32G32B32_SFLOAT;
    case fg::VT_VEC4:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
      break;
  }
}

VkVertexInputRate ToVk(fg::VertexInputRate rate) {
  switch (rate) {
    case fg::VertexInputRate::Vertex:
      return VK_VERTEX_INPUT_RATE_VERTEX;
    case fg::VertexInputRate::Instance:
      return VK_VERTEX_INPUT_RATE_INSTANCE;
      break;
  }
}
VkFilter ToVk(fg::Filter filter) {
  switch (filter) {
    case fg::Filter::Linear:
      return VK_FILTER_LINEAR;
    case fg::Filter::Nearest:
      return VK_FILTER_NEAREST;
  }
}

VkSamplerAddressMode ToVk(fg::SamplerAddressMode mode) {
  switch (mode) {
    case fg::SamplerAddressMode::Repeat:
      return VK_SAMPLER_ADDRESS_MODE_REPEAT;
      break;
    case fg::SamplerAddressMode::MirroredRepeat:
      return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case fg::SamplerAddressMode::ClampToEdge:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case fg::SamplerAddressMode::ClampToBorder:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
      break;
  }
}

VkBorderColor ToVk(fg::SamplerBorderColor color) {
  switch (color) {
    case fg::SamplerBorderColor::FloatTransparentBlack:
      return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    case fg::SamplerBorderColor::IntTransparentBlack:
      return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    case fg::SamplerBorderColor::FloatOpaqueBlack:
      return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    case fg::SamplerBorderColor::IntOpaqueBlack:
      return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    case fg::SamplerBorderColor::FloatOpaqueWhite:
      return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    case fg::SamplerBorderColor::IntOpaqueWhite:
      return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    case fg::SamplerBorderColor::CustomFloat:
      return VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;
    case fg::SamplerBorderColor::CustomInt:
      return VK_BORDER_COLOR_INT_CUSTOM_EXT;
  }
}
VkCompareOp ToVk(fg::CompareOperation op) {
  switch (op) {
    case fg::CompareOperation::Always:
      return VK_COMPARE_OP_ALWAYS;
    case fg::CompareOperation::Never:
      return VK_COMPARE_OP_NEVER;
    case fg::CompareOperation::Less:
      return VK_COMPARE_OP_LESS;
    case fg::CompareOperation::Equal:
      return VK_COMPARE_OP_EQUAL;
    case fg::CompareOperation::NotEqual:
      return VK_COMPARE_OP_NOT_EQUAL;
    case fg::CompareOperation::LessOrEqual:
      return VK_COMPARE_OP_LESS_OR_EQUAL;
    case fg::CompareOperation::Greater:
      return VK_COMPARE_OP_GREATER;
    case fg::CompareOperation::GreaterOrEqual:
      return VK_COMPARE_OP_GREATER_OR_EQUAL;
      break;
  }
}

ResourceState VkImageLayoutToResouceState(VkImageLayout layout) {
  switch (layout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
      return ResourceState::Undefined;
    case VK_IMAGE_LAYOUT_GENERAL:
      return ResourceState::Unknown;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      return ResourceState::RenderTarget;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      return ResourceState::DepthWrite;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
      return ResourceState::DepthRead;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      return ResourceState::ShaderResource;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      return ResourceState::CopySource;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      return ResourceState::CopyDest;
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      FOO_ASSERT(false, "Unexpected layout");
      return ResourceState::Undefined;
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
      FOO_ASSERT(false, "Unexpected layout");
      return ResourceState::Undefined;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
      FOO_ASSERT(false, "Unexpected layout");
      return ResourceState::Undefined;
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      return ResourceState::Present;
    default:
      FOO_ASSERT(false, "Unexpected layout");
      return ResourceState::Undefined;
  }
}

bool ResourceStateHasWriteAccess(ResourceState state) {
  switch (state) {
    case ResourceState::RenderTarget:
    case ResourceState::CopyDest:
      return true;
    default:
      return false;
  }
}
VkImageLayout ResourceStateToVkImageLayout(ResourceState state) {
  switch (state) {
    case ResourceState::Unknown:
    case ResourceState::Undefined:
      return VK_IMAGE_LAYOUT_UNDEFINED;
    case ResourceState::Vertex_buffer:
    case ResourceState::UniformBuffer:
    case ResourceState::IndexBuffer:
      FOO_ASSERT(false, "Unexpected state");
      return VK_IMAGE_LAYOUT_UNDEFINED;
    case ResourceState::RenderTarget:
      return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ResourceState::DepthWrite:
      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ResourceState::DepthRead:
      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case ResourceState::InputAttachment:
    case ResourceState::ShaderResource:
      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case ResourceState::CopyDest:
      return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ResourceState::CopySource:
      return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case ResourceState::Present:
      return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      break;
  }
}

VkPipelineStageFlags ResourceStateFlagsToVkPipelineStageFlags(ResourceState state) {
  switch (state) {
    case ResourceState::Unknown:
      FOO_ASSERT(false, "Unexpected");
    case ResourceState::Undefined:
      return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    case ResourceState::Vertex_buffer:
    case ResourceState::IndexBuffer:
      return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    case ResourceState::UniformBuffer:
    case ResourceState::ShaderResource:
      return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    case ResourceState::RenderTarget:
      return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case ResourceState::DepthWrite:
      return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    case ResourceState::DepthRead:
      return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    case ResourceState::CopyDest:
    case ResourceState::CopySource:
      return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case ResourceState::InputAttachment:
      return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    case ResourceState::Present:
      return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      break;
  }
}
}  // namespace fg
