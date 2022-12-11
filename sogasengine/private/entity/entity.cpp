#include "entity.h"

namespace Sogas
{
    DECL_OBJ_MANAGER( "entity", CEntity );

    CEntity::~CEntity()
    {
        for(u32 i = 0; i < CHandleManager::GetNumberDefinedTypes(); ++i)
        {
            if(Components[i].IsValid())
                Components[i].Destroy();
        }
    }

    void CEntity::DebugInMenu()
    {

    }

    void CEntity::RenderDebug()
    {

    }

    void CEntity::Set(u32 componentType, CHandle newComponent)
    {
        SASSERT(newComponent.IsValid());
        SASSERT(componentType < CHandle::MaxTypes);
        SASSERT(componentType > 0);
        SASSERT(Components[componentType].IsValid() == false)

        Components[componentType] = newComponent;
        newComponent.SetOwner(CHandle(this));
    }

    void CEntity::Set(CHandle newComponent)
    {
        Set(newComponent.GetType(), newComponent);
    }

    void CEntity::Load(const json& j)
    {

        for(const auto& it : j.items())
        {
            auto& component_name = it.key();
            auto& component_value = it.value();

            auto objectManager = CHandleManager::GetByName(component_name.c_str());

            if(!objectManager)
            {
                SFATAL("Object Manager '%s' not valid.", component_name.c_str());
                continue;
            }

            u32 component_type = objectManager->GetType();

            CHandle component_handle = Components[component_type];

            if(component_handle.IsValid())
            {
                component_handle.Load(component_value);
            }
            else
            {
                component_handle = objectManager->CreateHandle();

                Set(component_type, component_handle);

                component_handle.Load(component_value);
            }
        }
    }

    void CEntity::OnEntityCreated()
    {
        for(u32 i = 0; i < CHandleManager::GetNumberDefinedTypes(); ++i)
        {
            CHandle handle = Components[i];
            handle.OnEntityCreated();
        }
    }

    const char* CEntity::GetName() const
    {
        // TODO create TCompName
        return "<Unnamed>";
    }
}