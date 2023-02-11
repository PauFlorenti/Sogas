#pragma once

#include "modules/module_manager.h"

namespace Sogas
{
    class CEntityModule;
    class CRenderModule;

    const std::string search_paths[] = {
        "../data/",
        "../data/shaders/",
        "../../data/",
        "../../data/shaders/"
    };

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

        static const std::string FindFile(std::string filename);

        bool Init();
        void DoFrame();
        void Shutdown();

        CRenderModule* GetRenderModule() { return RenderModule; }

    private:
        static CEngine* engine;
        CRenderModule* RenderModule = nullptr;
        CEntityModule* EntityModule = nullptr;
        CModuleManager ModuleManager;

        void update(const f32 dt);
    };
} // Sogas
