#pragma once

#include "commandbuffer.h"
#include "handle/handle.h"
#include "render_types.h"

namespace Sogas
{
class CMesh;
class Material;

class CRenderManager
{
  public:
    struct Key
    {
        const CMesh*    Mesh;
        const Material* Material;
        CHandle         Owner;
        CHandle         Transform;
        CHandle         AABB;

        bool RenderInMenu();
    };

    using VKeys = std::vector<Key>;

    CRenderManager();

    void AddKey(CHandle owner, const CMesh* mesh, const Material* InMaterial);
    //void RenderAll(CHandle camera, Renderer::DrawChannel channel, Renderer::CommandBuffer cmd);
    void DeleteKeysFromOwner(CHandle owner);

  private:
    std::vector<u32> DrawCallsPerChannel;
    VKeys            keys;
    bool             KeysAreDirty = false;
};

extern CRenderManager RenderManager;

} // namespace Sogas
