#pragma once

#include "render_device.h"
#include "render_types.h"
#include "resource.h"

namespace Sogas
{
using namespace Renderer;
class CMesh : public IResource
{
  public:
    bool Create(std::vector<VertexLayout> vertices, std::vector<u32> indices, PrimitiveTopology topology);

    void Activate(CommandBuffer cmd) const;
    void Render(CommandBuffer cmd) const;

    void Destroy();

    u32 vertexCount  = 0;
    u32 vertexOffset = 0;
    u32 indexCount   = 0;
    u32 indexOffset  = 0;

    std::shared_ptr<Buffer> vertexBuffer;
    std::shared_ptr<Buffer> indexBuffer;
    PrimitiveTopology       Topology = PrimitiveTopology::UNDEFINED;
    bool                    Indexed  = false;

    void*                     data;
    std::weak_ptr<GPU_device> device;

  private:
    std::string         name;
    std::vector<VertexLayout> vertices;
    std::vector<u32>    indices;
    // AABB
    // Mesh group
};
} // namespace Sogas
