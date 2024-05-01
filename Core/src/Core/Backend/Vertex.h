#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <glm/glm.hpp>
namespace FooGame
{
    struct Vertex
    {
            glm::vec3 Position;
            glm::vec4 Color;
            glm::vec2 TexCoord;
    };

    struct QuadVertex
    {
            glm::vec3 Position;
            glm::vec4 Color;
            glm::vec2 TexCoord;
            // float TexIndex;
            // float TilingFactor;
            static VkVertexInputBindingDescription GetBindingDescription()
            {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding   = 0;
                bindingDescription.stride    = sizeof(QuadVertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                return bindingDescription;
            }
            static std::array<VkVertexInputAttributeDescription, 3>
            GetAttributeDescrp()
            {
                std::array<VkVertexInputAttributeDescription, 3>
                    attributeDescriptions{};
                attributeDescriptions[0].binding  = 0;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].offset =
                    offsetof(QuadVertex, Position);

                attributeDescriptions[1].binding  = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                attributeDescriptions[1].offset = offsetof(QuadVertex, Color);

                attributeDescriptions[2].binding  = 0;
                attributeDescriptions[2].location = 2;
                attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[2].offset =
                    offsetof(QuadVertex, TexCoord);
                return attributeDescriptions;
            }
    };
}  // namespace FooGame
