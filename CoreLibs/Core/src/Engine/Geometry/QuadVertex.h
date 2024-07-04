#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

namespace FooGame
{

    struct QuadVertex
    {
            glm::vec3 Position;
            glm::vec4 Color;
            glm::vec2 TexCoord;
            float TexIndex;
            float TilingFactor;
            static VkVertexInputBindingDescription GetBindingDescription()
            {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding   = 0;
                bindingDescription.stride    = sizeof(QuadVertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                return bindingDescription;
            }
            static std::vector<VkVertexInputAttributeDescription>
            GetAttributeDescriptionList()
            {
                std::vector<VkVertexInputAttributeDescription>
                    attributeDescriptions{};
                attributeDescriptions.resize(5);

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

                attributeDescriptions[3].binding  = 0;
                attributeDescriptions[3].location = 3;
                attributeDescriptions[3].format   = VK_FORMAT_R32_SFLOAT;
                attributeDescriptions[3].offset =
                    offsetof(QuadVertex, TexIndex);

                attributeDescriptions[4].binding  = 0;
                attributeDescriptions[4].location = 4;
                attributeDescriptions[4].format   = VK_FORMAT_R32_SFLOAT;
                attributeDescriptions[4].offset =
                    offsetof(QuadVertex, TilingFactor);
                return attributeDescriptions;
            }
            static std::array<VkVertexInputAttributeDescription, 5>
            GetAttributeDescrp()
            {
                std::array<VkVertexInputAttributeDescription, 5>
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

                attributeDescriptions[3].binding  = 0;
                attributeDescriptions[3].location = 3;
                attributeDescriptions[3].format   = VK_FORMAT_R32_SFLOAT;
                attributeDescriptions[3].offset =
                    offsetof(QuadVertex, TexIndex);

                attributeDescriptions[4].binding  = 0;
                attributeDescriptions[4].location = 4;
                attributeDescriptions[4].format   = VK_FORMAT_R32_SFLOAT;
                attributeDescriptions[4].offset =
                    offsetof(QuadVertex, TilingFactor);
                return attributeDescriptions;
            }
    };
}  // namespace FooGame
