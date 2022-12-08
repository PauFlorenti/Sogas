
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

    void CModuleManager::Update(f32 dt)
    {
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

    void CModuleManager::ParseModules(const std::string filename)
    {
        UpdateModules.clear();
        RenderModules.clear();

        // TODO Make a func to parse a json file.
        json j;
        std::ifstream ifs(filename.c_str());

        if(!ifs.is_open())
        {
            throw std::runtime_error("Failed to open json file.");
        }

        try
        {
            j = json::parse(ifs);
        }
        catch(const std::exception& e)
        {
            ifs.close();
            std::cerr << e.what() << "\n";
            throw std::runtime_error("Failed to parse json file.");
        }
        
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
}   // Sogas
