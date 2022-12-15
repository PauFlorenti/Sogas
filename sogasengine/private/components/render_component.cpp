#include "render_component.h"
#include "render/render_manager.h"
#include "resources/mesh.h"
#include "resources/resource.h"

namespace Sogas
{

    DECL_OBJ_MANAGER("render", TCompRender);

    bool TCompRender::DrawCall::Load(const json& j)
    {
        mesh = CResourceManager::Get()->GetResource(j["mesh"])->As<CMesh>();
        return true;
    }

    void TCompRender::RemoveFromRenderManager()
    {
        CHandle handle(this);
        RenderManager.DeleteKeysFromOwner(handle);
    }

    TCompRender::~TCompRender()
    {
        RemoveFromRenderManager();
    }

    void TCompRender::Load(const json& j)
    {
        if(j.is_array())
        {
            for(auto& jitem : j.items())
            {
                const json& jdc = jitem.value();
                DrawCall dc;
                if(dc.Load(jdc))
                    DrawCalls.push_back(dc);
            }
        }
    }

    void TCompRender::RenderDebug()
    {

    }

    void TCompRender::OnEntityCreated()
    {
        UpdateRenderManager();
    }

    void TCompRender::UpdateRenderManager()
    {
        RemoveFromRenderManager();

        CHandle handle(this);
        for(auto& dc : DrawCalls)
        {
            if(!dc.enabled)
                continue;
            RenderManager.AddKey(handle, dc.mesh);
        }
    }

} // Sogas
