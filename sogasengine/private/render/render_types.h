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
    };

} // Sogas
