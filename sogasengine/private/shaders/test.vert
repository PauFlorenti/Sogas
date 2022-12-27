#version 450

layout(location = 0) in vec3 InPosition;
layout(location = 1) in vec4 InColor;

layout( push_constant ) uniform constants
{
    mat4 model;
} model;

layout(location = 0) out vec4 OutColor;

void main() {
    mat4 model = model.model;
    gl_Position = model * vec4(InPosition, 1.0);
    OutColor = InColor;
}