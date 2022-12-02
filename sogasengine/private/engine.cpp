#include "engine.h"

#include "render/module_render.h"
#include "resources/primitives.h"

namespace Sogas
{
    CEngine* CEngine::engine = nullptr;

    CEngine::CEngine()
    {

    }

    bool CEngine::Init()
    {
        std::cout << "Initializing Engine ... \n";

        RenderModule = new CRenderModule("render");

        ModuleManager.RegisterServiceModule(RenderModule);

        ModuleManager.Boot();

        //! Move somewhere that fits
        RegisterPrimitives();

        return true;
    }

    void CEngine::DoFrame()
    {
        update(0.0f);
        RenderModule->DoFrame();
    }

    void CEngine::Shutdown()
    {
        RenderModule->Stop();
    }

    void CEngine::update(const f32 dt)
    {
        // dt should be const
        ModuleManager.Update(dt);
    }

} // Sogas
