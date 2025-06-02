#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(set = 0, binding = 0) uniform UBO{
    mat4 Model;
    mat4 View;
    mat4 Proj;
}ubo;


layout(location = 0) out vec3 fragColor;
void main() {
    // gl_Position = vec4(inPosition, 0.0, 1.0);

    gl_Position = ubo.Proj * ubo.View * ubo.Model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
