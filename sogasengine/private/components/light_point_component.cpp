#include "light_point_component.h"
#include "transform_component.h"

// TODO should not be referenced here.
#include "engine.h"
#include "render/module_render.h"
#include "render_device.h"

namespace Sogas
{
    DECL_OBJ_MANAGER("point_light", TCompPointLight);

    struct Light
    {
        glm::vec4 color;
        glm::vec3 position;
        f32 intensity;
        f32 radius;
    };

    void TCompPointLight::Load(const json& j)
    {
        intensity   = j.value("intensity", intensity);
        radius      = j.value("radius", radius);
        color       = LoadColor(j, "color");
        enabled     = j.value("enabled", enabled);
    }

    // bool TCompPointLight::Activate(const std::shared_ptr<Renderer::Buffer>& InBuffer, const u32 InLightNumber, Renderer::CommandBuffer cmd)
    // {
    //     if (intensity == 0.0f || enabled == false)
    //         return false;

    //     TCompTransform* transform = Get<TCompTransform>();
    //     SASSERT(transform);

    //     Light l;
    //     l.color     = color;
    //     l.intensity = intensity;
    //     l.position  = transform->GetPosition();
    //     l.radius    = radius;

    //     CEngine::Get()->GetRenderModule()->GetGraphicsDevice()->UpdateBuffer(InBuffer, &l, sizeof(Light), sizeof(Light) * InLightNumber, cmd);

    //     return true;
    // }

    const glm::vec3 TCompPointLight::GetPosition()
    {
        TCompTransform* transform = Get<TCompTransform>();
        SASSERT(transform);
        return transform->GetPosition();
    }
} // Sogas
