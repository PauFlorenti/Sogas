#pragma once

#include "modules/module.h"
#include "entity/entity.h"

namespace Sogas
{

    class CEntityModule : public IModule
    {
    public:

        CEntityModule(const std::string& name) : IModule(name) {}

        bool Start() override;
        void Stop() override;
        void Update(f32 /*dt*/) override;
        void Render() override ;
        void RenderDebug() override ;
        void RenderInMenu() override ;
        void RenderUI() override ;
        void RenderUIDebug() override ;

    private:

        std::vector<CHandleManager*> ObjectManagerToUpdate;
        std::vector<CHandleManager*> ObjectManagerToDebug;

        void LoadListOfManagers(const json& j, std::vector<CHandleManager*>& managers);
    };

} // Sogas
