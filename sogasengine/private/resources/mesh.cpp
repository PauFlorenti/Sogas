#include "engine.h"
#include "render/module_render.h"
#include "resources/mesh.h"

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
        this->vertexCount = static_cast<u32>(vs.size());

        GPUBufferDescriptor vertexBufferDescriptor;
        vertexBufferDescriptor.bindPoint    = BindPoint::VERTEX;
        vertexBufferDescriptor.usage        = Usage::UPLOAD;
        vertexBufferDescriptor.size         = static_cast<u64>(vs.size() * sizeof(Vertex));
        device.lock()->CreateBuffer(&vertexBufferDescriptor, vs.data(), &vertexBuffer);

        if (!is.empty())
        {
            Indexed = true;
            this->indexCount = static_cast<u32>(is.size());
            GPUBufferDescriptor indexBufferDescriptor;
            indexBufferDescriptor.bindPoint = BindPoint::INDEX;
            indexBufferDescriptor.usage     = Usage::UPLOAD;
            indexBufferDescriptor.size      = static_cast<u64>(indices.size() * sizeof(u32));
            device.lock()->CreateBuffer(&indexBufferDescriptor, is.data(), &indexBuffer);
        }

        return true;
    }

    void CMesh::Activate(CommandBuffer cmd) const
    {
        device.lock()->BindVertexBuffer(&vertexBuffer, cmd);

        if (Indexed)
            device.lock()->BindIndexBuffer(&indexBuffer, cmd);
    }

    void CMesh::Render(CommandBuffer cmd) const
    {
        if (Indexed)
            device.lock()->DrawIndexed(static_cast<u32>(indices.size()), indexOffset, cmd);
        else
            device.lock()->Draw(static_cast<u32>(vertices.size()), vertexOffset, cmd);
    }
} // Sogas
