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
namespace ShaderStages {

enum Stages {
  Vertex = 0x00000001,
  TessellationControl = 0x00000002,
  TessellationEvaluation = 0x00000004,
  Geometry = 0x00000008,
  Fragment = 0x00000010,
  Compute = 0x00000020,
  All_graphics = 0x0000001f,
  All = 0x7fffffff,
  Raygen = 0x00000100,
  AnyHit = 0x00000200,
  ClosestHit = 0x00000400,
  Miss = 0x00000800,
  Intersection = 0x00001000,
  Callable = 0x00002000,
  TaskExt = 0x00000040,
  MeshExt = 0x00000080,
  SubpassShadingHuawei = 0x00004000,
  ClusterCullingHuawei = 0x00080000,
  RaygenNv = Raygen,
  AnyHitNv = AnyHit,
  ClosestHitNv = ClosestHit,
  MissNv = Miss,
  IntersectionNv = Intersection,
  CallableNv = Callable,
  TaskNv = TaskExt,
  MeshNv = MeshExt,
  Flag_bits_max_enum = 0x7fffFFFF
};

}  // namespace ShaderStages

enum class DescriptorType {
  Sampler,
  CombinedImageSampler,
  SampledImage,
  UniformBuffer,
  UniformTexelBuffer,
  UniformBufferDynamic,
  StorageBuffer,
  InputAttachment,
};
enum ValueType : uint8_t {
  VT_FLOAT = sizeof(float),
  VT_VEC2 = VT_FLOAT * 2,
  VT_VEC3 = VT_FLOAT * 3,
  VT_VEC4 = VT_FLOAT * 4,

  VT_UINT = sizeof(uint32_t),
  VT_IVEC2 = VT_UINT * 2,
  VT_IVEC3 = VT_UINT * 3,
  VT_IVEC4 = VT_UINT * 4,

  VT_DOUBLE = sizeof(double),
};

enum class VertexInputRate : uint8_t {
  Vertex = 0,
  Instance,
};

// Vertex input binding and attribute description
struct VertexAttribute {
    uint8_t Binding = 0, Location = 0;
    ValueType Type;
};
enum class Filter {
  Nearest,
  Linear,
};
enum class SamplerAddressMode {
  Repeat,
  MirroredRepeat,
  ClampToEdge,
  ClampToBorder,
};
enum class SamplerBorderColor {
  FloatTransparentBlack,
  IntTransparentBlack,
  FloatOpaqueBlack,
  IntOpaqueBlack,
  FloatOpaqueWhite,
  IntOpaqueWhite,
  CustomFloat,
  CustomInt,
};
enum class CompareOperation {
  Never,
  Less,
  Equal,
  LessOrEqual,
  Greater,
  NotEqual,
  GreaterOrEqual,
  Always,
};
}  // namespace fg
