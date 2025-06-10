#pragma once
#include "../Core/Ref.h"
#include "VulkanObject.h"
#include "Misc.h"

namespace fg {

class VulkanBuffer;
class VulkanImage;
class VulkanImageView;
class VulkanGraphicsPipeline;

struct ShaderVariableDesc {
    std::string Name;
    uint32_t Binding = 0;
    uint32_t Set = 0;
    DescriptorType Type;
    ShaderStages::Stages ShaderStages;
    uint32_t Count = 1;
};

class ShaderVariable {
  public:
    ShaderVariable(const ShaderVariableDesc& desc, VkDescriptorSet set, Renderer* renderer,
                   VkSampler sampler)
        : m_Desc(desc), m_Set(set), m_Renderer(renderer), m_Sampler(sampler) {
    }

    void SetBuffer(VulkanBuffer* buffer);
    void SetImageView(VulkanImageView* image, uint32_t dstArrayElement);

    const ShaderVariableDesc& GetDesc() const {
      return m_Desc;
    }

  private:
    ShaderVariableDesc m_Desc;
    VkDescriptorSet m_Set;
    Renderer* m_Renderer;
    VkSampler m_Sampler;
};

class VulkanDescriptorSet : public RefBase {
  public:
    VulkanDescriptorSet(ReferenceCounter* counter, VulkanGraphicsPipeline* pipeline,
                        Renderer* renderer);

    ~VulkanDescriptorSet();
    ShaderVariable* GetByIndex(uint32_t);
    ShaderVariable* GetByName(const std::string& name);

    VkDescriptorSet GetHandle() const {
      return m_Set;
    }

  private:
    std::vector<ShaderVariable> m_Vars;
    DescriptorSetWrapper m_Set;
    Renderer* m_Renderer;
    VulkanGraphicsPipeline* m_Pipeline;
};

}  // namespace fg
