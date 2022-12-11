#pragma once

#include "base_component.h"
#include "resources/camera.h"

namespace Sogas
{
    class TCompCamera : public TCompBase, public CCamera
    {
        // DECL_SIBILING_ACCESS();

    public:
        void Load(const json& j);
        void Update(const f32 dt);
    };
    
} // Sogas
