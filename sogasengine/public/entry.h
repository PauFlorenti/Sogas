#pragma once

#include "application.h"

namespace Sogas
{
    bool GameEntry()
    {
        if(!CApplication::Get()->Init())
            SFATAL("Failed to initiate application.");

        CApplication::Get()->Run();

        CApplication::Get()->Shutdown();

        return true;
    }

} // Sogas
