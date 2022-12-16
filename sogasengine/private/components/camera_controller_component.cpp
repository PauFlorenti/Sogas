#include "application.h"
#include "camera_controller_component.h"
#include "name_component.h"
#include "transform_component.h"

namespace Sogas
{
    DECL_OBJ_MANAGER("camera_controller", CompCameraController);

    void CompCameraController::Load(const json& j)
    {
        speed = j.value("speed", speed);
    }

    void CompCameraController::Update(f32 dt)
    {
        TCompTransform* transform = Get<TCompTransform>();
        SASSERT(transform);

        f32 movement_speed = speed * dt;

        if (glfwGetKey(CApplication::Get()->GetWindow(), GLFW_KEY_W))
        {
            transform->SetPosition(transform->GetPosition() + movement_speed * transform->GetForward());
        }
        else if (glfwGetKey(CApplication::Get()->GetWindow(), GLFW_KEY_A))
        {
            transform->SetPosition(transform->GetPosition() - movement_speed * transform->GetRight());
        }
        else if (glfwGetKey(CApplication::Get()->GetWindow(), GLFW_KEY_S))
        {
            transform->SetPosition(transform->GetPosition() - movement_speed * transform->GetForward());
        }
        else if (glfwGetKey(CApplication::Get()->GetWindow(), GLFW_KEY_D))
        {
            transform->SetPosition(transform->GetPosition() + movement_speed * transform->GetRight());
        }
    }

} // Sogas
