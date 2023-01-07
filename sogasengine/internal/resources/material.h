#pragma once

#include "resource.h"

namespace Sogas
{
    class Texture;
    class Material : public IResource
    {
        const Texture* albedo               = nullptr;
        const Texture* normal               = nullptr;
        const Texture* metallic_roughness   = nullptr;
        const Texture* emissive             = nullptr;

    public:

    }; 
} // Sogas
