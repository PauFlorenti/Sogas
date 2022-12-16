#include "engine.h"
#include "mesh.h"
#include "render/module_render.h"

namespace Sogas
{
    bool CMesh::Create(std::vector<Vertex> vertices, PrimitiveTopology topology)
    {
        SASSERT( !vertices.empty() );
        SASSERT( topology != PrimitiveTopology::UNDEFINED );
        Indexed = false;
        Topology = topology;

        return CEngine::Get()->GetRenderModule()->CreateMesh(this, vertices, topology);
    }

    bool CMesh::Create(std::vector<Vertex> vertices, std::vector<u32> indices, PrimitiveTopology topology)
    {
        SASSERT( !vertices.empty() );
        SASSERT( topology != PrimitiveTopology::UNDEFINED );
        Topology    = topology;
        Indexed     = true;

        return CEngine::Get()->GetRenderModule()->CreateMesh(this, vertices, indices, topology);
    }

    void CMesh::Activate() const
    {
        CEngine::Get()->GetRenderModule()->Bind(this);
    }

    void CMesh::Render() const
    {
        CEngine::Get()->GetRenderModule()->Draw(this);
    }
} // Sogas
