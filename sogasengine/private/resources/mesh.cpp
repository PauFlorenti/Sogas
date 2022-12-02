#include "engine.h"
#include "mesh.h"
#include "render/module_render.h"
#include "render/vulkan/render.h"

namespace Sogas
{
    bool CMesh::Create(std::vector<Vertex> vertices, PrimitiveTopology topology)
    {
        assert(!vertices.empty());
        Indexed = false;
        Topology = topology;

        return CEngine::Get()->GetRenderModule()->CreateMesh(this, vertices, topology);
    }

    bool CMesh::Create(std::vector<Vertex> vertices, std::vector<u32> indices, PrimitiveTopology topology)
    {
        assert(!vertices.empty());
        assert(topology != PrimitiveTopology::UNDEFINED);
        Topology = topology;

        Indexed = true;

        return true;
    }

    void CMesh::Activate() const
    {
        CEngine::Get()->GetRenderModule()->Bind(RenderId, Topology);
    }

    void CMesh::Render() const
    {
        assert(RenderId != INVALID_ID);
        if( Indexed )
        {
            CEngine::Get()->GetRenderModule()->DrawIndexed(std::move(indexCount), std::move(indexOffset));
        }
        else
        {
            CEngine::Get()->GetRenderModule()->Draw(std::move(vertexCount), std::move(vertexOffset));
        }
    }
} // Sogas
