#version 450
layout(location = 0 ) in vec3 inPosition;
layout(location =1 ) in vec4 inColor;
layout(location =2 ) in vec2 inTexCoord;
layout(location =3 ) in float inTexIndex;
layout(location =4 ) in float inTilingFactor;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 outTexCoord;

layout(std140,binding = 0) uniform Projection{
	mat4 model;
	mat4 view;
	mat4 proj;
} proj;
vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);
void main()
{
	gl_Position = proj.model * proj.view * proj.proj * vec4(inPosition,1.0);
	fragColor = inColor;
	outTexCoord = inTexCoord;
}
