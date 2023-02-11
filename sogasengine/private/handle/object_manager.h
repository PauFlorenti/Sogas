#pragma once

#include "handle_manager.h"

namespace Sogas
{
    template< class TObj >
    class CObjectManager : public CHandleManager
    {
        std::vector<u8> AllocatedMemory;

        TObj* Objects = nullptr;

        void CreateObject(u32 InternalIndex) override
        {
            TObj* AddressToUse = Objects + InternalIndex;
            new (AddressToUse) TObj; // Call constructor in the address
        }

        void DestroyObject(u32 InternalIndex) override
        {
            TObj* AddressToUse = Objects + InternalIndex;
            AddressToUse->~TObj(); // Call destructor, not delete
        }

        void MoveObject(u32 SrcIndex, u32 DstIndex) override
        {
            TObj* src = Objects + SrcIndex;
            TObj* dst = Objects + DstIndex;
            new(dst) TObj(std::move(*src));
        }

        void LoadObject(u32 srcInternalIndex, const json& j) override
        {
            TObj* AddressToLoad = Objects + srcInternalIndex;
            AddressToLoad->Load(j);
        }

        void DebugInMenuObject(u32 internalIndex) override
        {
            TObj* address = Objects + internalIndex;
            address->DebugInMenu();
        }

        void RenderDebugObject(u32 internalIndex) override
        {
            TObj* address = Objects + internalIndex;
            address->RenderDebug();
        }

        void OnEntityCreateObject(u32 internalIndex) override
        {
            TObj* address = Objects + internalIndex;
            address->OnEntityCreated();
        }

    public:
        CObjectManager(const CObjectManager&) = delete;

        CObjectManager(const char* NewName) : Objects(nullptr)
        {
            Name = NewName;

            CHandleManager::PredefinedManagers[CHandleManager::nPredefinedManagers] = this;
            CHandleManager::nPredefinedManagers++;
        }

        void Init(u32 MaxObjects) override
        {
            CHandleManager::Init(MaxObjects);
            AllocatedMemory.resize(MaxObjects * sizeof(TObj));
            Objects = static_cast<TObj*>((void*)AllocatedMemory.data());
        }

        TObj* GetAddress()
        {
            return Objects;
        }

        CHandle GetHandleFromAddress(TObj* ObjectAddress)
        {
            auto InternalIndex = ObjectAddress - Objects;

            if(InternalIndex >= nObjectsUsed || InternalIndex < 0)
                return CHandle();

            auto externalIndex = InternalToExternal[InternalIndex];
            auto& externalData = ExternalToInternal[externalIndex];
            return CHandle(Type, externalIndex, externalData.CurrentAge);
        }

        TObj* GetAddressFromHandle(CHandle handle)
        {
            if(!handle.GetType())
                return nullptr;

            if (handle.GetType() != GetType())
            {
                SERROR("Requested to convert handle of type '%s' to a class of type '%s'.", 
                    CHandleManager::GetByType(handle.GetType())->GetName(), GetName());
                return nullptr;
            }

            const auto& externalData = ExternalToInternal[handle.GetExternalIndex()];

            if(externalData.CurrentAge != handle.GetAge())
                return nullptr;

            return Objects + externalData.InternalIndex;
        }

        void UpdateAll(f32 dt) override
        {
            SASSERT(Objects);

            if(!nObjectsUsed)
                return;

            for(u32 i = 0; i < nObjectsUsed; ++i)
                Objects[i].Update(dt);
        }

        void RenderDebugAll() override
        {
            SASSERT(Objects);

            for(u32 i = 0; i < nObjectsUsed; ++i)
                Objects[i].RenderDebug();
        }

        void DebugInMenuAll() override
        {
            SASSERT(Objects);

            for(u32 i = 0; i < nObjectsUsed; ++i)
            {
                Objects[i].DebugInMenu();
            }
        }

        template< typename TFn >
        void ForEach(TFn fn)
        {
            SASSERT(Objects);

            for(u32 i = 0; i < nObjectsUsed; ++i)
                fn(Objects + i);
        }
    };

    #define DECL_OBJ_MANAGER( object_name, object_class_name ) \
        CObjectManager< object_class_name > om_##object_class_name( object_name ); \
        template <> \
        CObjectManager< object_class_name >* GetObjectManager< object_class_name >() { return &om_##object_class_name; } \

} // Sogas
