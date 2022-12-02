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
            std::cout << "Engine failed to init.\n";
            return false;
        }

        return true;
    }

    void CApplication::InitInstance()
    {
        // Open Window ...
        std::cout << "Initializing application instance ...\n";
        std::cout << "\tInitializing GLFW ... \n";
        glfwInit();
        std::cout << "\tGLFW initialized.\n";

        std::cout << "\tCreating a GLFW window ...\n";
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(640, 480, "SogasEngine", nullptr, nullptr);
        std::cout << "\tGLFW window created.\n";
        std::cout << "Application instance initialized.\n\n";
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
        std::cout << "Shutting down ...\n";
        std::cout << "Terminating GLFW ...\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        std::cout << "GLFW terminated.\n";
    }
}
