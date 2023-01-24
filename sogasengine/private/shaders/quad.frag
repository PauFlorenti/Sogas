#version 450

layout(location = 0) in vec4 InColor;
layout(location = 1) in vec2 InUv;

layout(binding = 0) uniform sampler2D texture_color;

layout(location = 0) out vec4 OutColor;

void main()
{
    OutColor = InColor * texture(texture_color, InUv);
}