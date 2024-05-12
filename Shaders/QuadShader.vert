#version 450
layout(binding = 0) uniform UniformBufferObject {
	mat4 ViewProjection;
} ubo;

layout(location = 0 ) in vec3 inPosition;
layout(location = 1 ) in vec4 inColor;
layout(location = 2 ) in vec2 inTexCoord;
layout(location = 3 ) in float inTexIndex;
layout(location = 4 ) in float inTilingFactor;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out float outTexIndex;
layout(location = 3) out float outTilingFactor;

void main()
{
	gl_Position = ubo.ViewProjection * vec4(inPosition,1.0);
	outColor = inColor;
	outTexCoord = inTexCoord;
	outTexIndex = inTexIndex;
	outTilingFactor = inTilingFactor;
}
