#include "engine.h"

#include "modules/module_boot.h"
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
        STRACE("Initializing Engine ... ");
        static CModuleBoot boot("boot");

        RenderModule = new CRenderModule("render");

        ModuleManager.RegisterServiceModule(RenderModule);
        ModuleManager.RegisterGameModule(&boot);

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
