#pragma once

#include "modules/module.h"

namespace Sogas
{
    class CModuleBoot : public IModule
    {
    public:
        CModuleBoot(const std::string& name) : IModule(name) {}
        bool Start() override;
    private:
        void LoadScene(const std::string& filename);
    };
} // Sogas
