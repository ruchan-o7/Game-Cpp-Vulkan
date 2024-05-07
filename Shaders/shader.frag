#version 450
layout(binding = 1 ) uniform sampler2D texSampler;

layout(location = 0 ) in vec3 fragColor;
layout(location = 1 ) in vec2 texCoord;
//layout(location = 2 ) in float texIndex;
//layout(location = 3 ) in float texTilingFactor;

layout(location = 0 ) out vec4 outColor;

void main(){
	// outColor = vec4(fragColor);
	outColor = texture(texSampler,texCoord);
}
