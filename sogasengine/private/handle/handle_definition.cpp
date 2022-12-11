#include "handle_definition.h"
#include "handle_manager.h"

namespace Sogas
{

    void CHandle::Destroy()
    {
        auto handleManager = CHandleManager::GetByType(Type);
        handleManager->DestroyHandle(*this);
    }

    const char* CHandle::GetTypeName() const
    {
        auto handleManager = CHandleManager::GetByType(Type);
        if(handleManager)
            return handleManager->GetName();
        return "<invalid>";
    }

    CHandle CHandle::GetOwner() const
    {
        auto handleManager = CHandleManager::GetByType(Type);
        if(handleManager)
            return handleManager->GetOwner(*this);
        return CHandle();
    }

    void CHandle::SetOwner(CHandle newOwner)
    {
        SASSERT(newOwner.IsValid());

        auto handleManager = CHandleManager::GetByType(Type);
        if(handleManager)
            handleManager->SetOwner(*this, newOwner);
    }

    bool CHandle::IsValid() const 
    {
        auto handleManager = CHandleManager::GetByType(Type);
        return handleManager && handleManager->IsValid(*this);
    }

    void CHandle::RenderDebug()
    {
        auto handleManager = CHandleManager::GetByType(Type);
        if(handleManager)
            handleManager->RenderDebug(*this);
    }

    void CHandle::DebugInMenu()
    {
        auto handleManager = CHandleManager::GetByType(Type);
        if(handleManager)
            handleManager->DebugInMenu(*this);
    }

    void CHandle::Load(const json& j)
    {
        auto handleManager = CHandleManager::GetByType(Type);
        if(handleManager)
            handleManager->Load(*this, j);
    }

    void CHandle::OnEntityCreated()
    {
        auto handleManager = CHandleManager::GetByType(Type);
        if(handleManager)
            handleManager->OnEntityCreated(*this);
    }

} // Sogas
