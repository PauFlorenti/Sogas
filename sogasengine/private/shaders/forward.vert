#version 450

layout( set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 projection;
    mat4 view;
    mat4 view_projection;
    mat4 inverse_view_projection;
} Camera;

layout( push_constant ) uniform constants
{
    mat4 model;
} model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec3 fragColor;

void main() 
{
    mat4 model = model.model;

    gl_Position = Camera.projection * Camera.view * model * vec4(inPosition, 1.0);
    fragColor = inColor.xyz;
}