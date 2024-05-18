#pragma once
#include "../Defines.h"
#include <array>
#include <glm/glm.hpp>
struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;
namespace Engine
{
    struct Vertex
    {
            glm::vec3 Position;
            glm::vec3 Color;
            glm::vec2 TexCoord;
            static VkVertexInputBindingDescription GetBindingDescription();
            static std::array<VkVertexInputAttributeDescription, 3>
            GetAttributeDescrp();
            static List<VkVertexInputAttributeDescription>
            GetAttributeDescriptionList();
    };

}  // namespace Engine
