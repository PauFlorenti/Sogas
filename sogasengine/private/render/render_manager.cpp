
#include "components/transform_component.h"
#include "engine.h"
#include "entity/entity.h"
#include "render/module_render.h"
#include "render/render_manager.h"
#include "resources/mesh.h"

namespace Sogas
{
    CRenderManager RenderManager;

    struct ConstantsMesh
    {
        glm::mat4 model;
        glm::vec4 color;
    };

    CRenderManager::CRenderManager()
    {
        DrawCallsPerChannel.resize(static_cast<u32>(DrawChannel::COUNT));
    }

    void CRenderManager::AddKey(CHandle owner, const CMesh* mesh)
    {
        SASSERT(owner.IsValid());
        SASSERT(mesh);

        Key key;
        key.Mesh        = mesh;
        key.Owner       = owner;
        key.Transform   = CHandle();
        key.AABB        = CHandle();

        CEntity* entity = owner.GetOwner();
        SASSERT(entity);

        key.Transform = entity->Get<TCompTransform>();

        keys.push_back(key);
        KeysAreDirty = true;
    }

    void CRenderManager::RenderAll(CHandle /*camera_handle*/, DrawChannel channel, CommandBuffer cmd)
    {
        auto renderer = CEngine::Get()->GetRenderModule()->GetGraphicsDevice();

        // TODO If keys are dirty, sort keys

        u32 nDrawCalls = 0;
        DrawCallsPerChannel[static_cast<u32>(channel)] = 0;

        auto it = keys.begin();

        Key null_key = Key{};
        const Key* prev_key = &null_key;
        const Key* key = &null_key;

        while(it != keys.end())
        {
            key = &(*it);

            // Check if visible by culling test.

            if (key->Transform.IsValid())
            {
                CEntity* e = key->Owner.GetOwner();
                TCompTransform* transform = e->Get<TCompTransform>();
                SASSERT(transform);
                glm::vec4 color = glm::vec4(1.0f);

                // Activate model and color
                ConstantsMesh cte;
                cte.model = transform->AsMatrix();
                cte.color = color;
                
                renderer->PushConstants(&cte, sizeof(ConstantsMesh), cmd);
            }

            // Activate material

            // Activate mesh
            if( key->Mesh != prev_key->Mesh)
            {
                key->Mesh->Activate(cmd);
            }

            key->Mesh->Render(cmd);
            
            prev_key = key;
            ++nDrawCalls;
            ++it;
        }

        DrawCallsPerChannel[static_cast<u32>(channel)] = nDrawCalls;
    }

    void CRenderManager::DeleteKeysFromOwner(CHandle owner)
    {
        auto it = std::remove_if(keys.begin(), keys.end(), [owner](const Key& k){
            return k.Owner == owner;
        });
        keys.erase(it, keys.end());
    }
} // Sogas
