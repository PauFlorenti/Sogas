#pragma once

#include "resource.h"
#include "render/render_device.h"

namespace Sogas
{
    namespace Renderer
    {
        class Buffer;
    } // namespace Renderer
    class CMesh : public IResource
    {
    public:
        bool Create(
            std::vector<Vertex> vertices,
            std::vector<u32> indices,
            PrimitiveTopology topology);

        void Activate(CommandBuffer cmd) const;
        void Render(CommandBuffer cmd) const;

        u32 vertexCount = 0;
        u32 vertexOffset = 0;
        u32 indexCount = 0;
        u32 indexOffset = 0;

        std::unique_ptr<Renderer::Buffer> vertexBuffer;
        std::unique_ptr<Renderer::Buffer> indexBuffer;
        PrimitiveTopology Topology = PrimitiveTopology::UNDEFINED;
        bool Indexed = false;

        void *data;
        std::weak_ptr<GPU_device> device;

    private:
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
        // AABB
        // Mesh group
    };
} // namespace Sogas
