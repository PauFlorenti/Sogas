#pragma once

#include "base_component.h"
#include "entity/entity.h"

// TODO should not be used here
#include "render/render_types.h"
#include "render/buffer.h"

namespace Sogas
{
    class TCompPointLight : public TCompBase
    {
        DECL_SIBILING_ACCESS();
    public:
        glm::vec4   color{1.0f};
        glm::vec3   position{0.0f};
        bool        enabled{false};
        f32         radius{1.0f};
        f32         intensity{1.0f};

        void Load(const json& j);
        void DebutInMenu(){};
        void RenderDebug(){};
        bool Activate(const std::unique_ptr<Renderer::Buffer>& InBuffer, const u32 InLightNumber, CommandBuffer cmd);
        const glm::vec3 GetPosition();
    };
} // Sogas
