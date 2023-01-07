#pragma once

#include "handle/handle.h"
#include "render_types.h"

namespace Sogas
{

    class CMesh;

    class CRenderManager
    {
    public:
        struct Key
        {
            const CMesh* Mesh;
            CHandle Owner;
            CHandle Transform;
            CHandle AABB;

            bool RenderInMenu();
        };

        using VKeys = std::vector<Key>;

        CRenderManager();

        void AddKey(CHandle owner, const CMesh* mesh /*const CMaterial*/);
        void RenderAll(CHandle camera, DrawChannel channel, CommandBuffer cmd);
        void DeleteKeysFromOwner(CHandle owner);

    private:

        std::vector<u32> DrawCallsPerChannel;
        VKeys keys;
        bool KeysAreDirty = false;
    };

    extern CRenderManager RenderManager;

} // Sogas
