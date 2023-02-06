#pragma once

#include "resource.h"
#include "render_types.h"

namespace Sogas
{
    struct Texture;
    class Material : public IResource
    {
        const Texture* albedo               = nullptr;
        const Texture* normal               = nullptr;
        const Texture* metallic_roughness   = nullptr;
        const Texture* emissive             = nullptr;

    public:
        bool CreateFromJson(const json& j);

        void Activate(CommandBuffer cmd) const;
    }; 
} // Sogas
