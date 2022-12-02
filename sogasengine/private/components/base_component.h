#pragma once

namespace Sogas
{
    struct TCompBase
    {
        void DebugInMenu() {};
        void RenderDebug() {};
        void Load() {};
        void Update(f32 /*dt*/) {};
        void OnEntityCreated() {};
    };

    #define DECL_SIBILING_ACCESS()                  \
        template < typename TComp >                 \
        CHandle Get() {                             \
            CEntity* e = CHandle(this).GetOwner();  \
            if(!e)                                  \
                return CHandle();                   \
            return e->GetType();                    \
        }                                           \
                                                    \
        CEntity* GetEntity() {                      \
            CEntity* e = CHandle(this).GetOwner();  \
        }                                           \

} // Sogas
