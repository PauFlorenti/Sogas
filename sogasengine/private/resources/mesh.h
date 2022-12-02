#pragma once

#include "resource.h"
#include "render/render_types.h"

namespace Sogas
{
    class CMesh : IResource
    {
    public:
        bool Create(std::vector<Vertex> vertices, PrimitiveTopology topology);
        bool Create(std::vector<Vertex> vertices, std::vector<u32> indices, PrimitiveTopology topology);

        void Activate() const;
        void Render() const;

        u32 RenderId = INVALID_ID;
        u32 vertexCount = 0;
        u32 vertexOffset = 0;
        u32 indexCount = 0;
        u32 indexOffset = 0;

    private:
        u32 Id = INVALID_ID;
        std::string name;
        bool Indexed = false;
        PrimitiveTopology Topology;
        // AABB
        // Mesh group
    };
} // Sogas
