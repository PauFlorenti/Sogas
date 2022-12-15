
#include "components/transform_component.h"
#include "engine.h"
#include "entity/entity.h"
#include "render/module_render.h"
#include "render_manager.h"

namespace Sogas
{

    CRenderManager RenderManager;

    CRenderManager::CRenderManager()
    {
        DrawCallsPerChannel.resize(DrawChannel::COUNT);
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

        keys.push_back(key);
        KeysAreDirty = true;
    }

    void CRenderManager::RenderAll(CHandle /*camera_handle*/, DrawChannel channel)
    {
        // SASSERT(camera_handle.IsValid());

        // TODO If keys are dirty, sort keys

        // Retrieve active camera ...
        // CEntity* camera_entity = camera_handle;
        // SASSERT(camera_entity);

        // Of all keys, select only the channel range.

        u32 nDrawCalls = 0;
        DrawCallsPerChannel[channel] = 0;

        auto it = keys.begin();

        Key null_key = Key{};
        const Key* prev_key = &null_key;
        const Key* key = &null_key;

        while(it != keys.end())
        {
            key = &(*it);

            // Check if visible by culling test.

            //TCompTransform* transform = key->Transform;
            //SASSERT(transform);
            glm::vec4 color = glm::vec4(1.0f);

            glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

            // Activate model and color
            CEngine::Get()->GetRenderModule()->ActivateObject(model /*transform->AsMatrix()*/, color);

            // Activate material

            // Activate mesh
            if( key->Mesh != prev_key->Mesh)
                key->Mesh->Activate();

            key->Mesh->Render();
            
            prev_key = key;
            ++nDrawCalls;
            ++it;
        }

        DrawCallsPerChannel[channel] = nDrawCalls;
    }

    void CRenderManager::DeleteKeysFromOwner(CHandle owner)
    {
        auto it = std::remove_if(keys.begin(), keys.end(), [owner](const Key& k){
            return k.Owner == owner;
        });
        keys.erase(it, keys.end());
    }

} // Sogas
