
#include "modules/module_manager.h"

namespace Sogas
{
    void CModuleManager::Boot()
    {
        // Read from configuration file the boot modules.
        ParseModules(std::move(CEngine::FindFile("modules.json")));
        ParseGameStates(std::move(CEngine::FindFile("gamestates.json")));

        if(!BootGameState.empty())
        {
            // Change to game state.
            ChangeToGameState(BootGameState);
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
        if (ok)
        {
            Services.push_back(Module);
        }
    }

    void CModuleManager::RegisterGameModule(IModule* Module)
    {
        RegisteredModules[Module->GetName()] = Module;
    }

    void CModuleManager::RegisterGameState(const std::string& name, const VModules& modules)
    {
        RegisteredGameStates[name] = modules;
    }

    void CModuleManager::ChangeToGameState(const std::string& GameStateName)
    {
        auto it = RegisteredGameStates.find(GameStateName);

        SASSERT_MSG(it != RegisteredGameStates.cend(), "Trying to change to a non existent game state");
        
        if(it == RegisteredGameStates.cend()) return;

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
        for(auto module = Modules.rbegin(); module != Modules.rend(); ++module)
        {
            (*module)->DoStop();
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

        auto j = LoadJson(std::move(filename));
        
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

    void CModuleManager::ParseGameStates(const std::string& filename)
    {
        RegisteredGameStates.clear();

        json j = LoadJson(std::move(filename));

        BootGameState = j["boot"].get<std::string>();

        json jGameStateList = j["gamestates"];

        SASSERT(jGameStateList.size() > 0);

        for(auto jGameState : jGameStateList.items())
        {
            const std::string& jGameStateName = jGameState.key();

            GameState gs;
            for(auto jModule : jGameState.value())
            {
                const std::string& ModuleName = jModule.get<std::string>();
                IModule* module = GetModule(ModuleName);
                gs.push_back(module);
            }

            RegisterGameState(jGameStateName, gs);
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
