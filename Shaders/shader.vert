#version 450
layout(location = 0 ) in vec3 inPosition;
layout(location =1 ) in vec4 inColor;
layout(location =2 ) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 outTexCoord;

layout(binding = 0) uniform Projection{
	mat4 data;
} proj;

void main(){
	gl_Position = proj.data * vec4(inPosition,1.0);
	fragColor = inColor;
	outTexCoord = inTexCoord;
}
