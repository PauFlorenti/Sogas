#pragma once

#include "base_component.h"
#include "entity/entity.h"

namespace Sogas
{
    class CompCameraController : public TCompBase
    {
        DECL_SIBILING_ACCESS();

        f32 speed = 1.0f;

    public:
        void Load(const json& j);
        void Update(f32 dt);
    };
    
} // Sogas
