// #include "Renderer3D.h"
// #include "../Core/Base.h"
// #include <Engine.h>
// #include "../../Core/Graphics/Types/DescriptorData.h"
// #include <cassert>
// #include <unordered_map>
// #include <imgui.h>
// namespace FooGame
// {
//
// #if 1
// #define VERT_SHADER          "../../../Shaders/vert.spv"
// #define FRAG_SHADER          "../../../Shaders/frag.spv"
// #define DEFAULT_TEXTURE_PATH "../../../textures/texture.jpg"
// #else
//
// #define VERT_SHADER          "../../Shaders/vert.spv"
// #define FRAG_SHADER          "../../Shaders/frag.spv"
// #define MODEL_PATH           "../../Assets/Model/viking_room.obj"
// #define DEFAULT_TEXTURE_PATH "../../textures/texture.jpg"
// #endif
//
//     struct MeshPushConstants
//     {
//             glm::vec4 data;
//             glm::mat4 renderMatrix;
//     };
//     struct ModelDrawData
//     {
//             Buffer* VertexBuffer = nullptr;
//             Buffer* IndexBuffer  = nullptr;
//             Model* PtrModel      = nullptr;
//     };
//     struct StaticMeshContainer
//     {
//             List<Unique<Buffer>> VertexBuffer;
//             List<Unique<Buffer>> IndexBuffer;
//             size_t TotalSize = 0;
//             // todo some sort of data structure for keeping indexes offsets
//             etc
//     };
//
//     struct RenderData
//     {
//             struct Resources
//             {
//                     std::unordered_map<u32, ModelDrawData> ModelMap;
//                     u32 FreeIndex = 0;
//                     List<Buffer*> UniformBuffers;
//                     std::shared_ptr<Model> DefaultModel = nullptr;
//                     DescriptorData descriptor;
//                     vke::DescriptorAllocatorPool* DescriptorAllocatorPool;
//             };
//             struct FrameData
//             {
//                     u32 DrawCall = 0;
//             };
//             Resources Res;
//             FrameData FrameData;
//
//             struct Api
//             {
//                     Pipeline GraphicsPipeline;
//                     VkSampler TextureSampler;
//                     std::shared_ptr<Texture2D> DefaultTexture;
//             };
//             Api api{};
//     };
//     static RenderData s_Data;
//     static bool g_IsInitialized = false;
//     void Renderer3D::Init()
//     {
//         assert(!g_IsInitialized && "Do not init renderer3d twice!");
//         auto* device    = Api::GetDevice();
//         g_IsInitialized = true;
//         s_Data.Res.UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
//
//         for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
//         {
//             BufferBuilder uBuffBuilder{};
//             uBuffBuilder.SetUsage(BufferUsage::UNIFORM)
//                 .SetInitialSize(sizeof(UniformBufferObject))
//                 .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
//                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
//             s_Data.Res.UniformBuffers[i] = CreateDynamicBuffer(
//                 sizeof(UniformBufferObject), BufferUsage::UNIFORM);
//         }
//
//         s_Data.Res.DescriptorAllocatorPool =
//             vke::DescriptorAllocatorPool::Create(device->GetDevice());
//         auto allocator = s_Data.Res.DescriptorAllocatorPool->GetAllocator();
//         {
//             s_Data.Res.DescriptorAllocatorPool->SetPoolSizeMultiplier(
//                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT);
//             VkDescriptorSetLayoutBinding uboLayoutBinding{};
//             uboLayoutBinding.binding         = 0;
//             uboLayoutBinding.descriptorCount = 1;
//             uboLayoutBinding.descriptorType =
//             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//             uboLayoutBinding.pImmutableSamplers = nullptr;
//             uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
//             VkDescriptorSetLayoutBinding samplerLayoutBinding{};
//             samplerLayoutBinding.binding         = 1;
//             samplerLayoutBinding.descriptorCount = 1;
//             samplerLayoutBinding.descriptorType =
//                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             samplerLayoutBinding.pImmutableSamplers = nullptr;
//             samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
//             VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding,
//                                                         samplerLayoutBinding};
//             VkDescriptorSetLayoutCreateInfo layoutInfo{};
//             layoutInfo.sType =
//                 VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//             layoutInfo.bindingCount = ARRAY_COUNT(bindings);
//             layoutInfo.pBindings    = bindings;
//
//             VK_CALL(vkCreateDescriptorSetLayout(
//                 device->GetDevice(), &layoutInfo, nullptr,
//                 &s_Data.Res.descriptor.SetLayout));
//         }
//         {
//             s_Data.api.DefaultTexture = LoadTexture(DEFAULT_TEXTURE_PATH);
//         }
//         {
//             PipelineInfo info{};
//             Shader vert{VERT_SHADER, ShaderStage::VERTEX};
//             Shader frag{FRAG_SHADER, ShaderStage::FRAGMENT};
//             info.Shaders = List<Shader*>{&vert, &frag};
//             info.VertexAttributeDescriptons =
//                 Vertex::GetAttributeDescriptionList();
//             info.VertexBindings      = {Vertex::GetBindingDescription()};
//             info.LineWidth           = 2.0f;
//             info.CullMode            = CullMode::FRONT;
//             info.MultiSampling       = MultiSampling::LEVEL_1;
//             info.DescriptorSetLayout = s_Data.Res.descriptor.SetLayout;
//             info.pushConstantSize    = sizeof(MeshPushConstants);
//             info.pushConstantCount   = 1;
//
//             s_Data.api.GraphicsPipeline = CreateGraphicsPipeline(info);
//         }
//     }
//     void Renderer3D::SubmitModel(Model* model)
//     {
//         auto& mesh                                = model->GetMeshes()[0];
//         s_Data.Res.ModelMap[s_Data.Res.FreeIndex] = {
//             CreateVertexBuffer(mesh.m_Vertices),
//             CreateIndexBuffer(mesh.m_Indices), model};
//         auto device    = Api::GetDevice()->GetDevice();
//         auto allocator = s_Data.Res.DescriptorAllocatorPool->GetAllocator();
//         model->SetId(s_Data.Res.FreeIndex);
//         s_Data.Res.FreeIndex++;
//     }
//     void Renderer3D::BeginDraw()
//     {
//         s_Data.FrameData.DrawCall = 0;
//     }
//     void Renderer3D::EndDraw()
//     {
//         s_Data.Res.DescriptorAllocatorPool->Flip();
//     }
//     void Renderer3D::BeginScene(const PerspectiveCamera& camera)
//     {
//         UniformBufferObject ubd{};
//
//         ubd.View       = camera.GetView();
//         ubd.Projection = camera.GetProjection();
//         UpdateUniformData(ubd);
//     }
//
//     void Renderer3D::EndScene()
//     {
//     }
//
//     void Renderer3D::DrawModel(u32 id, const glm::mat4& transform)
//     {
//         auto currentFrame = Backend::GetCurrentFrame();
//         auto cmd          = Backend::GetCurrentCommandbuffer();
//         auto extent       = Backend::GetSwapchainExtent();
//
//         BindPipeline(cmd);
//
//         auto& modelRes = s_Data.Res.ModelMap[id];
//         auto* model    = modelRes.PtrModel;
//         auto& mesh     = model->GetMeshes()[0];
//
//         Api::SetViewportAndScissors(cmd, extent.width, extent.height);
//         VkBuffer vertexBuffers[] = {*modelRes.VertexBuffer->GetBuffer()};
//         VkDeviceSize offsets[]   = {0};
//         MeshPushConstants push{};
//         push.renderMatrix = transform;
//         auto allocator    =
//         s_Data.Res.DescriptorAllocatorPool->GetAllocator();
//         allocator.Allocate(mesh.GetLayout(), *mesh.GetSet(currentFrame));
//         vkCmdPushConstants(cmd, s_Data.api.GraphicsPipeline.pipelineLayout,
//                            VK_SHADER_STAGE_VERTEX_BIT, 0,
//                            sizeof(MeshPushConstants), &push);
//         vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
//
//         vkCmdBindIndexBuffer(cmd, *modelRes.IndexBuffer->GetBuffer(), 0,
//                              VK_INDEX_TYPE_UINT32);
//         BindDescriptorSets(cmd, mesh, s_Data.api.GraphicsPipeline);
//         vkCmdDrawIndexed(cmd, model->GetMeshes()[0].m_Indices.size(), 1, 0,
//         0,
//                          0);
//         s_Data.FrameData.DrawCall++;
//     }
//     void Renderer3D::BindDescriptorSets(VkCommandBuffer cmd, Mesh& mesh,
//                                         Pipeline& pipeLine,
//                                         VkPipelineBindPoint bindPoint,
//                                         u32 firstSet, u32 dSetCount,
//                                         u32 dynamicOffsetCount,
//                                         u32* dynamicOffsets)
//     {
//         auto currentFrame = Backend::GetCurrentFrame();
//         VkDescriptorBufferInfo bufferInfo{};
//         bufferInfo.buffer =
//             *s_Data.Res.UniformBuffers[currentFrame]->GetBuffer();
//         bufferInfo.offset = 0;
//         bufferInfo.range  = sizeof(UniformBufferObject);
//
//         VkWriteDescriptorSet descriptorWrites[2] = {};
//         descriptorWrites[0].sType      =
//         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; descriptorWrites[0].dstSet =
//         *mesh.GetSet(currentFrame); descriptorWrites[0].dstBinding = 0;
//         descriptorWrites[0].dstArrayElement = 0;
//         descriptorWrites[0].descriptorType  =
//         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//         descriptorWrites[0].descriptorCount = 1;
//         descriptorWrites[0].pBufferInfo     = &bufferInfo;
//
//         VkDescriptorImageInfo imageInfo{};
//         imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         imageInfo.imageView   = mesh.m_Texture->ImageView;
//         imageInfo.sampler     = mesh.m_Texture->Sampler;
//
//         descriptorWrites[1].sType      =
//         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; descriptorWrites[1].dstSet =
//         mesh.m_DescriptorSets[currentFrame]; descriptorWrites[1].dstBinding =
//         1; descriptorWrites[1].dstArrayElement = 0;
//         descriptorWrites[1].descriptorType =
//             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         descriptorWrites[1].descriptorCount = 1;
//         descriptorWrites[1].pImageInfo      = &imageInfo;
//
//         vkUpdateDescriptorSets(Api::GetDevice()->GetDevice(),
//                                ARRAY_COUNT(descriptorWrites),
//                                descriptorWrites, 0, nullptr);
//         vkCmdBindDescriptorSets(cmd, bindPoint, pipeLine.pipelineLayout, 0,
//         1,
//                                 mesh.GetSet(currentFrame), 0, nullptr);
//     }
//     void Renderer3D::BindPipeline(VkCommandBuffer cmd)
//     {
//         vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
//                           s_Data.api.GraphicsPipeline.pipeline);
//     }
//     void Renderer3D::SubmitScene(Scene* scene)
//     {
//     }
//     void Renderer3D::ReleaseScene(Scene* scene)
//     {
//     }
//
//     void Renderer3D::Shutdown()
//     {
//         auto device = Api::GetDevice()->GetDevice();
//         for (auto& [index, data] : s_Data.Res.ModelMap)
//         {
//             data.IndexBuffer->Release();
//             data.VertexBuffer->Release();
//         }
//         vkDestroyDescriptorPool(
//             device,
//             s_Data.Res.DescriptorAllocatorPool->GetAllocator().vkPool,
//             nullptr);
//
//         for (auto& ub : s_Data.Res.UniformBuffers)
//         {
//             ub->Release();
//             delete ub;
//         }
//         s_Data.Res.UniformBuffers.clear();
//         vkDestroyPipelineLayout(
//             device, s_Data.api.GraphicsPipeline.pipelineLayout, nullptr);
//         vkDestroyPipeline(device, s_Data.api.GraphicsPipeline.pipeline,
//                           nullptr);
//     }
//     void Renderer3D::UpdateUniformData(UniformBufferObject ubd)
//     {
//         s_Data.Res.UniformBuffers[Backend::GetCurrentFrame()]->SetData(
//             sizeof(ubd), &ubd);
//     }
//
// }  // namespace FooGame
