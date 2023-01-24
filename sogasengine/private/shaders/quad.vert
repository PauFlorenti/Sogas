#version 450

layout(location = 0) in vec3 InPosition;
layout(location = 1) in vec3 InNormal;
layout(location = 2) in vec2 InUv;
layout(location = 3) in vec4 InColor;

layout(location = 0) out vec4 OutColor;
layout(location = 1) out vec2 OutUv;

void main()
{
    OutColor = InColor;
    OutUv = InUv;

    gl_Position = vec4(InPosition, 1.0);
}