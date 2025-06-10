#include "VulkanDescriptorSet.h"
#include "TypeConversions.h"
#include "VulkanBuffer.h"
#include "Renderer.h"
#include "../Core/Assert.h"

namespace fg {

VulkanDescriptorSet::VulkanDescriptorSet(ReferenceCounter* counter,
                                         VulkanGraphicsPipeline* pipeline, Renderer* renderer)
    : RefBase(counter), m_Renderer(renderer), m_Pipeline(pipeline) {
  const auto& pipeDesc = pipeline->GetDesc();
  const auto& layouts = pipeline->GetSetLayouts();

  std::vector<VkDescriptorSetLayout> setLayouts;
  setLayouts.reserve(layouts.size());
  for (const auto& l : layouts) {
    setLayouts.emplace_back(l);
  }

  VkDescriptorSetAllocateInfo allocInfo {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = pipeline->GetPool();
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = setLayouts.data();

  auto device = m_Renderer->GetLogicalDevice();
  m_Set = device->AllocateDescriptorSet(allocInfo);

  for (const auto& var : pipeDesc.ShaderVariables) {
    VkDescriptorSet set = m_Set;
    m_Vars.emplace_back(var, set, m_Renderer, m_Pipeline->GetSampler());
  }
}

VulkanDescriptorSet::~VulkanDescriptorSet() {
  auto device = m_Renderer->GetLogicalDevice();
  device->FreeDescriptorSet(std::move(m_Set), m_Pipeline->GetPool());
}

ShaderVariable* VulkanDescriptorSet::GetByIndex(uint32_t idx) {
  FOO_ASSERT(idx < m_Vars.size());
  return &m_Vars[idx];
}

ShaderVariable* VulkanDescriptorSet::GetByName(const std::string& name) {
  for (auto& var : m_Vars) {
    if (var.GetDesc().Name == name) {
      return &var;
    }
  }
  return nullptr;
}

void ShaderVariable::SetBuffer(VulkanBuffer* buffer) {
  FOO_ASSERT(m_Renderer != nullptr);
  FOO_ASSERT(buffer != nullptr);

  VkDescriptorBufferInfo info {};
  info.buffer = buffer->GetVkBuffer();
  info.offset = 0;
  info.range = VK_WHOLE_SIZE;
  VkWriteDescriptorSet wds {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  wds.dstSet = m_Set;
  wds.dstBinding = m_Desc.Binding;
  wds.dstArrayElement = 0;
  wds.pBufferInfo = &info;
  wds.descriptorType = ToVk(m_Desc.Type);
  wds.descriptorCount = 1;
  auto device = m_Renderer->GetLogicalDevice();
  device->UpdateDescriptorSets(1, &wds, 0, nullptr);
}
void ShaderVariable::SetImageView(VulkanImageView* image, uint32_t dstArrayElement) {
  FOO_ASSERT(m_Renderer != nullptr);
  FOO_ASSERT(image != nullptr);

  VkDescriptorImageInfo info {};
  info.imageView = image->GetHandle();
  info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  info.sampler = m_Sampler;
  VkWriteDescriptorSet wds {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  wds.dstSet = m_Set;
  wds.dstBinding = m_Desc.Binding;
  wds.dstArrayElement = 0;
  wds.pImageInfo = &info;
  wds.descriptorType = ToVk(m_Desc.Type);
  wds.descriptorCount = 1;
  auto device = m_Renderer->GetLogicalDevice();
  device->UpdateDescriptorSets(1, &wds, 0, nullptr);
}
}  // namespace fg
