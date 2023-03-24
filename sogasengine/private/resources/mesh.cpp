#include "resources/mesh.h"
#include "engine.h"
#include "render/module_render.h"

namespace Sogas
{
bool CMesh::Create(std::vector<Renderer::VertexLayout> vs, std::vector<u32> is, PrimitiveTopology topology)
{
    device = CEngine::Get()->GetRenderModule()->GetGraphicsDevice();

    SASSERT(!vs.empty());
    SASSERT(topology != PrimitiveTopology::UNDEFINED);
    Topology          = topology;
    Indexed           = false;
    this->vertices    = vs;
    this->indices     = is;
    this->vertexCount = static_cast<u32>(vs.size());

    // Renderer::BufferDescriptor vertexBufferDescriptor;
    // vertexBufferDescriptor.binding     = Renderer::BufferBindingPoint::Vertex;
    // vertexBufferDescriptor.usage       = Renderer::BufferUsage::TRANSFER_DST;
    // vertexBufferDescriptor.size        = static_cast<u32>(vs.size());
    // vertexBufferDescriptor.elementSize = sizeof(Renderer::VertexLayout);
    // vertexBuffer                       = device.lock()->CreateBuffer(vertexBufferDescriptor, vs.data());

    // if (!is.empty())
    // {
    //     Indexed          = true;
    //     this->indexCount = static_cast<u32>(is.size());
    //     Renderer::BufferDescriptor indexBufferDescriptor;
    //     indexBufferDescriptor.binding     = Renderer::BufferBindingPoint::Index;
    //     indexBufferDescriptor.usage       = Renderer::BufferUsage::TRANSFER_DST;
    //     indexBufferDescriptor.size        = static_cast<u32>(indices.size());
    //     indexBufferDescriptor.elementSize = sizeof(u32);
    //     indexBuffer                       = device.lock()->CreateBuffer(indexBufferDescriptor, is.data());
    // }

    return true;
}

// void CMesh::Activate(CommandBuffer /*cmd*/) const
// {
//     // device.lock()->BindVertexBuffer(vertexBuffer, cmd);

//     // if (Indexed)
//     // {
//     //     device.lock()->BindIndexBuffer(indexBuffer, cmd);
//     // }
// }

// void CMesh::Render(CommandBuffer cmd) const
// {
//     if (Indexed)
//     {
//         device.lock()->DrawIndexed(static_cast<u32>(indices.size()), indexOffset, cmd);
//     }
//     else
//     {
//         device.lock()->Draw(static_cast<u32>(vertices.size()), vertexOffset, cmd);
//     }
// }

void CMesh::Destroy()
{
    // if (vertexBuffer && vertexBuffer->isValid())
    // {
    //     vertexBuffer->Release();
    // }
    // if (indexBuffer && indexBuffer->isValid())
    // {
    //     indexBuffer->Release();
    // }
}

} // namespace Sogas
