#pragma once
#include "../Core/Base.h"
#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
namespace FooGame
{
    struct Vertex
    {
            glm::vec3 Position;
            glm::vec3 Color;
            glm::vec2 TexCoord;
            // float TexIndex;
            // float TilingFactor;
            static VkVertexInputBindingDescription GetBindingDescription()
            {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding   = 0;
                bindingDescription.stride    = sizeof(Vertex);
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
                attributeDescriptions[0].offset   = offsetof(Vertex, Position);

                attributeDescriptions[1].binding  = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[1].offset   = offsetof(Vertex, Color);

                attributeDescriptions[2].binding  = 0;
                attributeDescriptions[2].location = 2;
                attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[2].offset   = offsetof(Vertex, TexCoord);

                // attributeDescriptions[3].binding  = 0;
                // attributeDescriptions[3].location = 3;
                // attributeDescriptions[3].format   = VK_FORMAT_R32_SFLOAT;
                // attributeDescriptions[3].offset   = offsetof(Vertex,
                // TexIndex);
                //
                // attributeDescriptions[4].binding  = 0;
                // attributeDescriptions[4].location = 4;
                // attributeDescriptions[4].format   = VK_FORMAT_R32_SFLOAT;
                // attributeDescriptions[4].offset =
                //     offsetof(Vertex, TilingFactor);
                return attributeDescriptions;
            }
            static List<VkVertexInputAttributeDescription>
            GetAttributeDescriptionList()
            {
                List<VkVertexInputAttributeDescription> attributeDescriptions{};
                attributeDescriptions.resize(3);
                attributeDescriptions[0].binding  = 0;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].offset   = offsetof(Vertex, Position);

                attributeDescriptions[1].binding  = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[1].offset   = offsetof(Vertex, Color);

                attributeDescriptions[2].binding  = 0;
                attributeDescriptions[2].location = 2;
                attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[2].offset   = offsetof(Vertex, TexCoord);

                // attributeDescriptions[3].binding  = 0;
                // attributeDescriptions[3].location = 3;
                // attributeDescriptions[3].format   = VK_FORMAT_R32_SFLOAT;
                // attributeDescriptions[3].offset   = offsetof(Vertex,
                // TexIndex);
                //
                // attributeDescriptions[4].binding  = 0;
                // attributeDescriptions[4].location = 4;
                // attributeDescriptions[4].format   = VK_FORMAT_R32_SFLOAT;
                // attributeDescriptions[4].offset =
                //     offsetof(Vertex, TilingFactor);
                return attributeDescriptions;
            }
    };

}  // namespace FooGame
