#include "engine.h"
#include "mesh.h"
#include "render/module_render.h"

namespace Sogas
{
    bool CMesh::Create(std::vector<Vertex> vs, std::vector<u32> is, PrimitiveTopology topology)
    {
        
        device = CEngine::Get()->GetRenderModule()->GetGraphicsDevice();

        SASSERT( !vs.empty() );
        SASSERT( topology != PrimitiveTopology::UNDEFINED );
        Topology        = topology;
        Indexed         = false;
        this->vertices  = vs;
        this->indices   = is;

        GPUBufferDescriptor vertexBufferDescriptor;
        vertexBufferDescriptor.bindPoint    = BindPoint::VERTEX;
        vertexBufferDescriptor.usage        = Usage::READBACK;
        vertexBufferDescriptor.size         = static_cast<u64>(vs.size() * sizeof(Vertex));
        device.lock()->CreateBuffer(&vertexBufferDescriptor, vs.data(), &vertexBuffer);

        if (!is.empty())
        {
            Indexed = true;
            GPUBufferDescriptor indexBufferDescriptor;
            indexBufferDescriptor.bindPoint = BindPoint::INDEX;
            indexBufferDescriptor.usage     = Usage::READBACK;
            indexBufferDescriptor.size      = static_cast<u64>(indices.size() * sizeof(u32));
            device.lock()->CreateBuffer(&indexBufferDescriptor, is.data(), &indexBuffer);
        }

        return true;
    }

    void CMesh::Activate() const
    {
        device.lock()->SetTopology(Topology);

        device.lock()->BindVertexBuffer(&vertexBuffer);

        if (Indexed)
            device.lock()->BindIndexBuffer(&indexBuffer);
    }

    void CMesh::Render() const
    {
        if (Indexed)
            device.lock()->DrawIndexed(static_cast<u32>(indices.size()), indexOffset);
        else
            device.lock()->Draw(static_cast<u32>(vertices.size()), vertexOffset);
    }
} // Sogas
