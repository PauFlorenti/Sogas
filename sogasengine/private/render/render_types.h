#pragma once

namespace Sogas 
{
    enum GraphicsAPI
    {
        Vulkan = 0,
        OpenGL = 1,
        Dx11 = 2,
        Dx12 = 3
    };

    enum PrimitiveTopology
    {
        UNDEFINED,
        TRIANGLELIST,
        TRIANGLESTRIP,
        TRIANGLEFAN,
        POINTLIST,
        LINELIST,
        LINESTRIP
    };

    struct Vertex
    {
        glm::vec3 position;
        //glm::vec3 normal;
        //glm::vec2 uvs;
        glm::vec4 color;

        bool operator==(const Vertex& other) const {
            return position == other.position && 
                //normal == other.normal &&
                //uvs == other.uvs &&
                color == other.color;
        }
    };

    enum DrawChannel
    {
        SOLID = 0,
        SHADOW_CASTER = 1,
        TRANSPARENT = 2,
        COUNT = TRANSPARENT
    };

} // Sogas
