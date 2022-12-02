#pragma once

#include "module.h"

namespace Sogas
{
    class CModuleManager
    {
    public:
        void Boot();
        void Clear();

        void Update(f32 dt);
        void Render();
        void RenderUI();
        void RenderInMenu();
        void RenderDebug();
        void RenderUIDebug();

        void RegisterServiceModule(IModule* Module);
        void RegisterGameModule(IModule* Module);

        IModule* GetModule(const std::string& ModuleName);

    private:
        void StartModules(VModules& Modules);
        void StopModules(VModules& Modules);

        void ParseModules(const std::string filename);

        VModules Services;
        VModules UpdateModules;
        VModules RenderModules;

        GameState* BootModules = nullptr;

        std::map<std::string, IModule*> RegisteredModules;
        std::map<std::string, GameState> registeredGameStates;
    };
}
