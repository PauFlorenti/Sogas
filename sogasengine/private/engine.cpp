#include "engine.h"

#include "modules/module_boot.h"
#include "modules/module_entities.h"
#include "render/module_render.h"
#include "resources/primitives.h"
#include "resources/mesh.h"
#include "resources/resource.h"

#include "GLFW/glfw3.h"

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

        CResourceManager::Get()->RegisterResourceType(GetResourceType<CMesh>());

        RenderModule = new CRenderModule("render");
        EntityModule = new CEntityModule("entities");

        ModuleManager.RegisterServiceModule(RenderModule);
        ModuleManager.RegisterServiceModule(EntityModule);
        ModuleManager.RegisterGameModule(&boot);

        ModuleManager.Boot();

        //! Move somewhere that fits
        RegisterPrimitives();

        return true;
    }

    void CEngine::DoFrame()
    {
        static f64 previousTime = 0.0;
        f64 currentTime = glfwGetTime();
        f64 elapsed = currentTime - previousTime;
        update(static_cast<f32>(elapsed));
        RenderModule->DoFrame();
        previousTime = currentTime;
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
