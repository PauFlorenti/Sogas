#pragma once

#include "resource.h"
#include "render/render_device.h"

namespace Sogas
{
    class CMesh : public IResource
    {
    public:
        bool Create(
            std::vector<Vertex> vertices, 
            std::vector<u32> indices, 
            PrimitiveTopology topology);

        void Activate() const;
        void Render() const;

        u32 vertexCount = 0;
        u32 vertexOffset = 0;
        u32 indexCount = 0;
        u32 indexOffset = 0;

        GPUBuffer vertexBuffer;
        GPUBuffer indexBuffer;
        PrimitiveTopology Topology = PrimitiveTopology::UNDEFINED;
        bool Indexed = false;

        void* data;
        std::weak_ptr<GPU_device> device;

    private:
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
        // AABB
        // Mesh group
    };
} // Sogas
