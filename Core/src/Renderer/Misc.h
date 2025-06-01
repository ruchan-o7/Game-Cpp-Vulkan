#pragma once
namespace fg {

enum class ResourceState : uint16_t {
  Unknown = 0,
  Undefined,
  Vertex_buffer,
  UniformBuffer,
  IndexBuffer,
  RenderTarget,
  DepthWrite,
  DepthRead,
  ShaderResource,
  CopyDest,
  CopySource,
  InputAttachment,
  Present,
};

}  // namespace fg
