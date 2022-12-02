#pragma once

#include "handle_definition.h"

namespace Sogas
{
    class CHandleManager
    {
        static const u32 MaxTotalObjectsAllowed = 1 << CHandle::nBitsIndex;

        struct ExternalData
        {
            u32 InternalIndex;
            u32 NextExternalIndex;
            CHandle CurrentOwner;
            u32 CurrentAge : CHandle::nBitsAge;

            ExternalData()
                : InternalIndex(0), NextExternalIndex(0), CurrentAge(0)
            {}
        };
        
    protected:
        u32 Type;
        std::vector<ExternalData> ExternalToInternal;
        std::vector<u32> InternalToExternal;

        u32 nObjectsUsed;
        u32 NextFreeHandleExternalIndex;
        u32 LastFreeHandleExternalIndex;

        std::vector<CHandle> ObjectsToDestroy;

        const char* Name = nullptr;

        // Shared by all managers
        static u32 NextTypeOfHandleManager;
        static CHandleManager* AllManagers[CHandle::MaxTypes];
        static std::map<std::string, CHandleManager*> AllManagersByName;

        virtual void CreateObject(u32 internalIndex) = 0;
        virtual void DestroyObject(u32 internalIndex) = 0;
        virtual void MoveObject(u32 srcInternalIndex, u32 dstInternalIndex) = 0;
        virtual void LoadObject(u32 srcInternalIndex, const json& j) = 0;
        virtual void DebugInMenuObject(u32 internalIndex) = 0;
        virtual void RenderDebugObject(u32 internalIndex) = 0;
        virtual void OnEntityCreateObject(u32 internalIndex) = 0;

    public:
        CHandleManager()
            : Type(0), nObjectsUsed(0), NextFreeHandleExternalIndex(0), LastFreeHandleExternalIndex(0)
        {}

        CHandleManager(const CHandleManager&) = delete;
        ~CHandleManager();

        virtual void Init(u32 maxObjects);
        bool IsValid(CHandle h) const;
        const char* GetName() { return Name; }
        u32 GetType() const { return Type; }
        u32 GetSize() const { return nObjectsUsed; }
        u32 GetCapacity() const { return static_cast<u32>(ExternalToInternal.size()); }

        bool DestroyPendingObjects();

        CHandle CreateHandle();
        void DestroyHandle(CHandle h);
        void DebugInMenu(CHandle h);
        void RenderDebug(CHandle h);
        void OnEntityCreated(CHandle h);
        void Load(CHandle h, const json& j);

        // Applies to all objects
        virtual void UpdateAll(f32 dt) = 0;
        virtual void RenderDebugAll() = 0;
        virtual void DebugInMenuAll() = 0;

        void SetOwner(CHandle who, CHandle newOwner);
        CHandle GetOwner(CHandle who);

        static CHandleManager* GetByType(const u32 type);
        static CHandleManager* GetByName(const char* objectName);
        static u32 GetNumberDefinedTypes();
        static void DestroyAllPendingObjects();
        void DumpInternals() const;
    };

} // Sogas
