#include "application.h"
#include "engine.h"

namespace Sogas
{
    CApplication* CApplication::app = nullptr;

    CApplication::CApplication()
    {
        bIsRunning = false;
        bQuitting = false;
    }

    // This should read a json configuration file
    bool CApplication::Init()
    {
        InitInstance();

        if(!CEngine::Get()->Init()){
            SERROR("Engine failed to init.");
            return false;
        }

        return true;
    }

    void CApplication::InitInstance()
    {
        // Open Window ...
        STRACE("Initializing application instance ...");
        STRACE("\tInitializing GLFW ... ");
        glfwInit();
        STRACE("\tGLFW initialized.");

        STRACE("\tCreating a GLFW window ...");
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(640, 480, "SogasEngine", nullptr, nullptr);
        STRACE("\tGLFW window created.");
        STRACE("Application instance initialized.\n");
    }

    void CApplication::Run()
    {
        while(!glfwWindowShouldClose(window)){
            glfwPollEvents();
            CEngine::Get()->DoFrame();
        }
    }

    void CApplication::Shutdown()
    {
        STRACE("Shutting down ... ");
        CEngine::Get()->Shutdown();
        STRACE("Terminating GLFW ... ");
        glfwDestroyWindow(window);
        glfwTerminate();
        STRACE("GLFW terminated. ");
    }
}
