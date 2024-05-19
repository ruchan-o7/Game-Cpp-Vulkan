#pragma once
#include "../Defines.h"
#include <array>
#include <vector>
#include <glm/glm.hpp>
struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;
namespace FooGame
{
    struct Vertex
    {
            glm::vec3 Position;
            glm::vec3 Color;
            glm::vec2 TexCoord;
            static VkVertexInputBindingDescription GetBindingDescription();
            static std::array<VkVertexInputAttributeDescription, 3>
            GetAttributeDescrp();
            static std::vector<VkVertexInputAttributeDescription>
            GetAttributeDescriptionList();
    };

}  // namespace FooGame
