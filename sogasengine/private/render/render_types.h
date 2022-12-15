#pragma once

namespace Sogas
{
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
        glm::vec3 Position;
        glm::vec4 Color;

        bool operator==(const Vertex& other) const {
            return Position == other.Position && Color == other.Color;
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
