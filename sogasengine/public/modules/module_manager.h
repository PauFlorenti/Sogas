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
        void RegisterGameState(const std::string& name, const VModules& modules);

        void ChangeToGameState(const std::string& GameStateName);
        void ChangeToGameState(GameState* newGameState);

        IModule* GetModule(const std::string& ModuleName);

    private:
        void StartModules(VModules& Modules);
        void StopModules(VModules& Modules);

        void ParseModules(const std::string& filename);
        void ParseGameStates(const std::string& filename);

        void ChangeToRequestedGameState();

        VModules Services;
        VModules UpdateModules;
        VModules RenderModules;

        std::string BootGameState;
        GameState* CurrentGameState = nullptr;
        GameState* RequestedGameState = nullptr;

        std::map<std::string, IModule*> RegisteredModules;
        std::map<std::string, GameState> RegisteredGameStates;
    };
}
