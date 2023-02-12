#include "handle_manager.h"

namespace Sogas
{
    u32                                     CHandleManager::NextTypeOfHandleManager = 1;
    CHandleManager*                         CHandleManager::AllManagers[CHandle::MaxTypes];
    std::map<std::string, CHandleManager*>  CHandleManager::AllManagersByName;

    CHandleManager* CHandleManager::PredefinedManagers[CHandle::MaxTypes];
    u32             CHandleManager::nPredefinedManagers = 0;
    bool CHandleManager::bHandleToDestroy = false;

    void CHandleManager::Init(u32 maxObjects)
    {
        SASSERT(maxObjects < MaxTotalObjectsAllowed);
        SASSERT(maxObjects > 0);

        // Register this as the handler with the new type.
        if(Type == 0)
        {
            Type = NextTypeOfHandleManager;
            NextTypeOfHandleManager++;
        }

        AllManagers[Type] = this;
        AllManagersByName[GetName()] = this;

        nObjectsUsed = 0;
        u32 nObjectsCapacity = maxObjects;

        ExternalToInternal.resize(nObjectsCapacity);
        InternalToExternal.resize(nObjectsCapacity);

        u32 i = 0;
        for(auto& ei : ExternalToInternal)
        {
            ei.CurrentAge = 1;
            ei.InternalIndex = INVALID_ID;
            if(i != nObjectsCapacity - 1)
                ei.NextExternalIndex = i + 1;
            else
                ei.NextExternalIndex = INVALID_ID;
            InternalToExternal.at(i) = INVALID_ID;
            ++i;
        }

        NextFreeHandleExternalIndex = 0;
        LastFreeHandleExternalIndex = nObjectsCapacity - 1;
    }

    bool CHandleManager::IsValid(CHandle h) const
    {
        SASSERT(h.GetType() == Type);
        SASSERT(h.GetExternalIndex() < GetCapacity());

        auto& ei = ExternalToInternal[h.GetExternalIndex()];
        return ei.CurrentAge == h.GetAge();
    }

    bool CHandleManager::DestroyPendingObjects()
    {
        for(auto object : ObjectsToDestroy)
        {
            auto externalIndex = object.GetExternalIndex();
            auto& externalData = ExternalToInternal[externalIndex];

            auto internalIndex = externalData.InternalIndex;

            SASSERT((nObjectsUsed > 0));

            externalData.CurrentAge--;

            DestroyObject(internalIndex);

            externalData.CurrentAge++;

            // Set owner to null.
            externalData.CurrentOwner = CHandle();

            SASSERT((LastFreeHandleExternalIndex != INVALID_ID));
            auto& lastFreeExternalData = ExternalToInternal[LastFreeHandleExternalIndex];
            SASSERT((lastFreeExternalData.NextExternalIndex == INVALID_ID));

            lastFreeExternalData.NextExternalIndex = externalIndex;
            LastFreeHandleExternalIndex = externalIndex;

            SASSERT((externalData.NextExternalIndex == INVALID_ID));

            // Move all object to the begining, so they are all compacted at the beginning.
            u32 internalIndexOfLastValidObject = nObjectsUsed - 1;
            if(internalIndex < internalIndexOfLastValidObject)
            {
                MoveObject(internalIndexOfLastValidObject, internalIndex);

                auto movedObjectExternalIndex = InternalToExternal[internalIndexOfLastValidObject];
                InternalToExternal[internalIndex] = movedObjectExternalIndex;

                auto& movedObjectExternalData = ExternalToInternal[movedObjectExternalIndex];
                movedObjectExternalData.InternalIndex = internalIndex;
            }

            nObjectsUsed--;
        }

        bool somethingDeleted = !ObjectsToDestroy.empty();

        ObjectsToDestroy.clear();

        return somethingDeleted;
    }

    CHandle CHandleManager::CreateHandle()
    {
        // Make sure I am valid.
        SASSERT(Type != 0);

        const u32 nObjectsCapacity = GetCapacity();

        SASSERT(NextFreeHandleExternalIndex != INVALID_ID);
        SASSERT(nObjectsUsed < nObjectsCapacity);
        u32 externalIndex = NextFreeHandleExternalIndex;
        auto& externalData = ExternalToInternal[externalIndex];

        externalData.InternalIndex = nObjectsUsed;
        
        SASSERT(externalData.CurrentOwner == CHandle());

        InternalToExternal[externalData.InternalIndex] = externalIndex;

        CreateObject(externalData.InternalIndex);

        ++nObjectsUsed;

        NextFreeHandleExternalIndex = externalData.NextExternalIndex;
        SASSERT(NextFreeHandleExternalIndex != INVALID_ID);

        externalData.NextExternalIndex = INVALID_ID;

        return CHandle(Type, externalIndex, externalData.CurrentAge);
    }

    void CHandleManager::DebugInMenu(CHandle h)
    {
        if(!h.IsValid())
            return;
        auto& externalData = ExternalToInternal[h.GetExternalIndex()];
        DebugInMenuObject(externalData.InternalIndex);
    }

    void CHandleManager::RenderDebug(CHandle h)
    {
        if(!h.IsValid())
            return;
        auto& externalData = ExternalToInternal[h.GetExternalIndex()];
        RenderDebugObject(externalData.InternalIndex);
    }

    void CHandleManager::OnEntityCreated(CHandle h)
    {
        if(!h.IsValid())
            return;
        auto& externalData = ExternalToInternal[h.GetExternalIndex()];
        OnEntityCreateObject(externalData.InternalIndex);
    }

    void CHandleManager::Load(CHandle h, const json& j)
    {
        if(!h.IsValid())
            return;
        
        auto& externalData = ExternalToInternal[h.GetExternalIndex()];
        LoadObject(externalData.InternalIndex, j);
    }

    void CHandleManager::SetOwner(CHandle who, CHandle newOwner)
    {
        SASSERT(who.IsValid());
        auto& externalData = ExternalToInternal[who.GetExternalIndex()];
        externalData.CurrentOwner = newOwner;
    }

    CHandle CHandleManager::GetOwner(CHandle who)
    {
        if(!who.IsValid())
            return CHandle();
        auto& externalData = ExternalToInternal[who.GetExternalIndex()];
        return externalData.CurrentOwner;
    }

    void CHandleManager::DestroyHandle(CHandle h)
    {
        if(!IsValid(h))
            return;

        ObjectsToDestroy.push_back(h);

        auto externalIndex = h.GetExternalIndex();
        auto& externalData = ExternalToInternal[externalIndex];
        externalData.CurrentAge++;
    }

    CHandleManager* CHandleManager::GetByType(const u32 type)
    {
        return AllManagers[type];
    }

    CHandleManager* CHandleManager::GetByName( const char* objectName )
    {
        auto it = AllManagersByName.find(objectName);
        if(it == AllManagersByName.end())
            return nullptr;
        return it->second;
    }

    u32 CHandleManager::GetNumberDefinedTypes()
    {
        return NextTypeOfHandleManager;
    }

    void CHandleManager::DestroyAllPendingObjects()
    {
        if (!bHandleToDestroy)
        {
            return;
        }
        
        bool bSomethingDeleted = false;
        do {
            bSomethingDeleted = false;

            // First type is CEntity type, avoid it.
            for (u32 i = 1; i < GetNumberDefinedTypes(); ++i)
            {
                bSomethingDeleted |= AllManagers[i]->DestroyPendingObjects();
            }
        }
        while (bSomethingDeleted);
        bSomethingDeleted = false;
    }

    void CHandleManager::DumpInternals() const
    {

    }

} // Sogas
