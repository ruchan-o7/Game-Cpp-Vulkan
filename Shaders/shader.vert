#version 450
layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0 ) in vec3 inPosition;
layout(location = 1 ) in vec3 inColor;
layout(location = 2 ) in vec2 inTexCoord;
//layout(location = 3 ) in float inTexIndex;
//layout(location = 4 ) in float inTilingFactor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 outTexCoord;
//layout(location = 2) out float outTexIndex;
//layout(location = 3) out float outTilingFactor;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition,1.0);
	fragColor = inColor;
	outTexCoord = inTexCoord;
//	outTexIndex = inTexIndex;
//	outTilingFactor = inTilingFactor;
}
