#pragma once

#include "application.h"

namespace Sogas
{
    bool GameEntry()
    {
        if(!CApplication::Get()->Init())
            std::cout << "Failed to initiate application.\n";

        CApplication::Get()->Run();

        CApplication::Get()->Shutdown();

        return true;
    }

} // Sogas
