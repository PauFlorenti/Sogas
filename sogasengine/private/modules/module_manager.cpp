
#include "modules/module_manager.h"

namespace Sogas
{
    void CModuleManager::Boot()
    {
        // Read from configuration file the boot modules.
        ParseModules("../../../sogasengine/private/modules.json");

        if(!BootModules)
        {
            // Change to game state.
            ChangeToGameState(BootModules);
        }
    }

    void CModuleManager::Clear()
    {
        // Set game state to nullptr.

        StopModules(Services);
        Services.clear();
    }

    void CModuleManager::RegisterServiceModule(IModule* Module)
    {
        RegisteredModules[Module->GetName()] = Module;

        const bool ok = Module->DoStart();
        if(!ok)
        {
            Services.push_back(Module);
        }
    }

    void CModuleManager::RegisterGameModule(IModule* Module)
    {
        RegisteredModules[Module->GetName()] = Module;
    }

    void CModuleManager::ChangeToGameState(const std::string& GameStateName)
    {
        auto it = registeredGameStates.find(GameStateName);

        SASSERT_MSG(it != registeredGameStates.cend(), "Trying to change to a non existent game state");
        
        if(it == registeredGameStates.cend()) return;

        ChangeToGameState(&(it->second));
    }

    void CModuleManager::ChangeToGameState(GameState* newGameState)
    {
        RequestedGameState = newGameState;
    }

    void CModuleManager::Update(f32 dt)
    {
        ChangeToRequestedGameState();

        for(auto module : UpdateModules)
        {
            if(module->IsActive())
            {
                module->Update(dt);
            }
        }
    }

    void CModuleManager::Render()
    {
        for(auto module : RenderModules)
        {
            if(module->IsActive())
            {
                module->Render();
            }
        }
    }

    void CModuleManager::RenderUI()
    {
        for(auto module : RenderModules)
        {
            if(module->IsActive())
            {
                module->RenderUI();
            }
        }
    }

    void CModuleManager::RenderUIDebug()
    {
        for(auto module : RegisteredModules)
        {
            if(module.second->IsActive())
            {
                module.second->RenderUIDebug();
            }
        }
    }

    void CModuleManager::RenderDebug()
    {
        for(auto module : RegisteredModules)
        {
            if(module.second->IsActive())
            {
                module.second->RenderDebug();
            }
        }
    }

    void CModuleManager::StartModules(VModules& Modules)
    {
        for(auto module : Modules)
        {
            module->DoStart();
        }
    }

    void CModuleManager::StopModules(VModules& Modules)
    {
        for(auto module : Modules)
        {
            module->DoStop();
        }
    }

    IModule* CModuleManager::GetModule(const std::string& filename)
    {
        auto moduleIt = RegisteredModules.find(filename);
        return moduleIt != RegisteredModules.end() ? moduleIt->second : nullptr;
    }

    void CModuleManager::ParseModules(const std::string& filename)
    {
        UpdateModules.clear();
        RenderModules.clear();

        auto j = LoadJson(filename);
        
        json jUpdateList = j["update"];
        json jRenderList = j["render"];

        for(auto jModule : jUpdateList)
        {
            const std::string& moduleName = jModule.get<std::string>();
            IModule* module = GetModule(moduleName);
            SASSERT((module != nullptr));
            if(module)
            {
                UpdateModules.push_back(module);
            }
        }

        for(auto jModule : jRenderList)
        {
            const std::string& moduleName = jModule.get<std::string>();
            IModule* module = GetModule(moduleName);
            SASSERT( module != nullptr );
            if(module)
            {
                RenderModules.push_back(module);
            }
        }
    }

    void CModuleManager::ChangeToRequestedGameState()
    {
        if(RequestedGameState == nullptr || RequestedGameState == CurrentGameState)
            return;

        if(CurrentGameState)
        {
            StopModules(*CurrentGameState);
            CurrentGameState = nullptr;
        }

        if(RequestedGameState)
        {
            StartModules(*RequestedGameState);
            CurrentGameState = RequestedGameState;
            RequestedGameState = nullptr;
        }
    }

}   // Sogas
