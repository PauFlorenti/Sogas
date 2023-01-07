#pragma once

namespace Sogas
{
    class GPU_device;

    enum BufferBindingPoint
    {
        Vertex = 0,
        Index = 1,
        Uniform = 2
    };

    enum BufferType
    {
        Static = 0,
        Dynamic = 1
    };
} // Sogas
