#version 450

layout(location = 0) in vec3 InPosition;
layout(location = 1) in vec3 InNormal;
layout(location = 2) in vec2 InUv;
layout(location = 3) in vec4 InColor;

layout(location = 0) out vec4 OutColor;
layout(location = 1) out vec3 OutNormal;
layout(location = 2) out vec2 OutUv;
layout(location = 3) out vec3 OutPosition;

layout( set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 projection;
    mat4 view;
    mat4 view_projection;
    mat4 inverse_view_projection;
} Camera;

layout ( binding = 1 ) uniform MeshObject
{
    mat4 model;
    vec4 color;
} mesh;

/*
layout( push_constant ) uniform constants
{
    mat4 model;
    vec4 color;
} MeshConstants;
*/

void main() 
{
    mat4 model          = mesh.model;
    vec4 color          = mesh.color;
    vec3 worldPosition  = vec4(model * vec4(InPosition, 1.0)).xyz;

    gl_Position = Camera.projection * Camera.view * vec4(worldPosition, 1.0);
    OutColor    = color * InColor;
    OutNormal   = mat3(transpose(inverse(model))) * InNormal;
    OutUv       = InUv;
    OutPosition = worldPosition;
}