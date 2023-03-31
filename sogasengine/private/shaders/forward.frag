#version 450

layout(location = 0) in vec4 InColor;
layout(location = 1) in vec3 InNormal;
layout(location = 2) in vec2 InUv;
layout(location = 3) in vec3 InPosition;

const int MAX_LIGHTS = 2;

struct Light {
    vec3    color;
    float   intensity;
    vec3    position;
    float   radius;
};

layout(set = 0, binding = 2) uniform Lights
{
    Light light[MAX_LIGHTS];
} LightUniform;

// layout(set = 1, binding = 0) uniform sampler2D diffuse;
// layout(set = 1, binding = 1) uniform sampler2D normal;
// layout(set = 1, binding = 2) uniform sampler2D metallic_roughness;
// layout(set = 1, binding = 3) uniform sampler2D emissive;

layout(location = 0) out vec4 OutColor;

const vec3 white = vec3(1.0);

void main() 
{
    vec3    N                   = normalize(InNormal);
    vec3    diffuse_color       = white; //InColor.xyz * texture(diffuse, InUv).xyz;
    vec3    normal_color        = white; //texture(normal, InUv).xyz;
    vec3    metallic_roughness  = white; //texture(metallic_roughness, InUv).xyz;
    vec3    emissive            = white; //texture(emissive, InUv).xyz;
    float   ambientLight        = 0.1;

    vec3 light = vec3(ambientLight);

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        Light l = LightUniform.light[i];
        vec3 L = normalize(l.position - InPosition);
        float NdotL = max(dot(InNormal, L), 0.0);

        light += NdotL * l.color.xyz;
    }

    OutColor = vec4(diffuse_color * light, 1.0);
}