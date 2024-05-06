// #pragma once
// #include "../Core/Base.h"
// #include <glm/glm.hpp>
// #include <vulkan/vulkan.h>
// #include "../Graphics/Camera.h"
// namespace FooGame
// {
//     class Renderer2D
//     {
//         public:
//             static void Init();
//             static void Shutdown();
//             static void BeginScene(const Camera& camera);
//             static void EndDraw();
//             static void Flush();
//             static void SetClearColor(glm::vec3 color);
//             static void DrawQuad(const glm::vec2& position,
//                                  const glm::vec2& size, const glm::vec4&
//                                  color);
//             static void DrawQuad(const glm::vec3& position,
//                                  const glm::vec2& size, const glm::vec4&
//                                  color);
//             // static void DrawQuad(const glm::vec2& position,
//             //                      const glm::vec2& size,
//             //                      const Ref<Texture2D>& texture,
//             //                      float tilingFactor         = 1.0f,
//             //                      const glm::vec4& tintColor =
//             //                      glm::vec4(1.0f));
//             // static void DrawQuad(const glm::vec3& position,
//             //                      const glm::vec2& size,
//             //                      const Ref<Texture2D>& texture,
//             //                      float tilingFactor         = 1.0f,
//             //                      const glm::vec4& tintColor =
//             //                      glm::vec4(1.0f));

//             static void DrawQuad(const glm::mat4& transform,
//                                  const glm::vec4& color);
//             // static void DrawQuad(const glm::mat4& transform,
//             //                      const Ref<Texture2D>& texture,
//             //                      float tilingFactor         = 1.0f,
//             //                      const glm::vec4& tintColor =
//             //                      glm::vec4(1.0f), int entityID = -1);

//             static void Resize();
//             static void ResetStats();

//             static VkDevice GetDevice();
//             struct Statistics
//             {
//                     u32 DrawCalls = 0;
//                     u32 QuadCount = 0;
//                     u32 GetTotalVertexCount() const { return QuadCount * 4; }
//                     u32 GetTotalIndex() const { return QuadCount * 6; }
//             };

//         private:
//             static void StartBatch();
//     };

// }  // namespace FooGame
