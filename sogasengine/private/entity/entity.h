#pragma once

#include "components/base_component.h"

namespace Sogas
{
    class CEntity : public TCompBase
    {
        CHandle Components[CHandle::MaxTypes];

    public:
        ~CHandle();

        CHandle Get(u32 ComponentType) const 
        {
            assert(ComponentType < CHandle::MaxTypes);
            return Components[ComponentType];
        }

        template < typename TComp >
        CHandle Get() const 
        {

        }

        void DebugInMenu();
        void RenderDebug();

        void Set(u32 componentType, CHadnle newComponent);
        void Set(CHandle newComponent);
        void Load(const json& j);
        void OnEntityCreated();

        const char* GetName() const;
    }
}