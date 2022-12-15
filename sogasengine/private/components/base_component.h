#pragma once

namespace Sogas
{
    struct TCompBase
    {
        void DebugInMenu() {};
        void RenderDebug() {};
        void Load(const json& /*j*/) {};
        void Update(f32 /*dt*/) {};
        void OnEntityCreated() {};
    };

    #define DECL_SIBILING_ACCESS()                  \
        template < typename TComp >                 \
        CHandle Get() {                             \
            CEntity* e = CHandle(this).GetOwner();  \
            if(!e)                                  \
                return CHandle();                   \
            return e->Get<TComp>();                 \
        }                                           \
                                                    \
        CEntity* GetEntity() {                      \
            CEntity* e = CHandle(this).GetOwner();  \
            return e;                               \
        }                                           \

} // Sogas
