#pragma once

#include "modules/module_manager.h"

namespace Sogas
{
    class CRenderModule;

    class CEngine
    {
    public:
        CEngine();
        ~CEngine(){};

        static CEngine* Get()
        {
            if(!engine)
                engine = new CEngine();
            return engine;
        }

        bool Init();
        void DoFrame();
        void Shutdown();

        CRenderModule* GetRenderModule() { return RenderModule; }

    private:
        static CEngine* engine;
        CRenderModule* RenderModule = nullptr;
        CModuleManager ModuleManager;

        void update(const f32 dt);
    };
} // Sogas
