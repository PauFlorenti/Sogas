#include "module_entities.h"

namespace Sogas
{
    bool CModuleEntities::Start() 
    { 
        json j = LoadJson("../../../sandbox/data/components.json");

        std::map<std::string, u32> ComponentSizes = j["sizes"];
        i32 defaultSize = ComponentSizes["default"];

        std::map<std::string, u32> initOrder = j["init_order"];

        std::sort(
            CHandleManager::PredefinedManagers, 
            CHandleManager::PredefinedManagers + CHandleManager::nPredefinedManagers, 
            [&initOrder](CHandleManager* m1, CHandleManager* m2)
            {
                const i32 priority1 = initOrder[m1->GetName()];
                const i32 priority2 = initOrder[m2->GetName()];
                return priority1 > priority2;
            });

        SASSERT(strcmp(CHandleManager::PredefinedManagers[0]->GetName(), "entity") == 0);

        for(u32 i = 0; i < CHandleManager::nPredefinedManagers; ++i)
        {
            const auto& objectManager = CHandleManager::PredefinedManagers[i];
            const auto& iterator = ComponentSizes.find(objectManager->GetName());
            i32 size = (iterator == ComponentSizes.end()) ? defaultSize : iterator->second;
            STRACE("Initializing object manager '%s' with size '%d'.", objectManager->GetName(), size);
            objectManager->Init(size);
        }

        LoadListOfManagers(j["update"], ObjectManagerToUpdate);
        // TODO render debug managers ...

        return true; 
    }

    void CModuleEntities::Stop()
    {

    }

    void CModuleEntities::Update(f32 /*dt*/) {} 
    void CModuleEntities::Render() {}
    void CModuleEntities::RenderDebug() {}
    void CModuleEntities::RenderInMenu() {}
    void CModuleEntities::RenderUI() {}
    void CModuleEntities::RenderUIDebug() {}

    void CModuleEntities::LoadListOfManagers(const json& j, std::vector<CHandleManager*>& managers)
    {
        managers.clear();

        std::vector<std::string> names = j;
        
        for(const auto& n : names)
        {
            CHandleManager* objectManager = CHandleManager::GetByName(n.c_str());
            SASSERT_MSG(objectManager, "Cannot find a valid object manager.");
            managers.push_back(objectManager);
        }
    }
} // Sogas
