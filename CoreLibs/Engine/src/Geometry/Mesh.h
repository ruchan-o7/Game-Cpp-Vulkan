#pragma once
#include <cstdint>
#include <vector>
#include "Vertex.h"
#include <memory>
#include "Material.h"
#include "../Engine/Texture2D.h"
namespace FooGame {

  struct Primitive {
	  uint32_t FirstIndex;
	  uint32_t IndexCount;
	  int32_t  MaterialIndex;
  };
  struct Mesh {
	public:
	  Mesh();
	  ~Mesh();
	  Mesh(Mesh&&);

	  Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);

	  Mesh(const std::vector<Vertex>&   vertices,
		   const std::vector<uint32_t>& indices);
	  std::vector<Vertex>    m_Vertices;
	  std::vector<uint32_t>  m_Indices;
	  std::vector<Primitive> MeshPrimitives;
	  Material               materialData;
	  uint32_t               RenderId;

	  void SetTexture(const std::shared_ptr<Texture2D>& texture) {
		m_Texture = texture;
	  }

	  VkDescriptorSetLayout GetLayout() const { return m_Layout; }
	  VkDescriptorSet*      GetSet(uint32_t index) {
        return &m_DescriptorSets[index];
	  }
	  std::vector<VkDescriptorSet>& GetSets() { return m_DescriptorSets; }
	  std::vector<VkDescriptorSet>  m_DescriptorSets {3};
	  VkDescriptorSetLayout         m_Layout;
	  std::shared_ptr<Texture2D>    m_Texture;
  };

}  // namespace FooGame
