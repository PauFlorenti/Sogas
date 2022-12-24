#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Sogas
{
    class CApplication
    {
    protected:

        static CApplication* app;

        GLFWwindow* window = nullptr;

        bool bIsRunning;
        bool bQuitting;

    public:

        CApplication();
        virtual ~CApplication(){};

        static CApplication* Get()
        {
            if(!app)
                app = new CApplication();
            return app;
        };

        GLFWwindow* GetWindow() { return window; }
        bool IsRunning() { return bIsRunning; }
        void GetWindowSize(i32* width, i32* height) { SASSERT(window); glfwGetWindowSize(window, width, height);}

        virtual bool Init();
        virtual void Run();
        virtual void Shutdown();

    private:
        virtual void InitInstance();
    };
} // Sogas
