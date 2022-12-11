#pragma once
#include "base_component.h"

namespace Sogas
{
    class TCompTransform : public TCompBase
    {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;

    public:       
        void SetPosition(const glm::vec3& new_position) { position = new_position; }
        void SetRotation(const glm::quat& new_rotation) { rotation = new_rotation; }
        void SetScale(const glm::vec3& new_scale) { scale = new_scale; }

        glm::vec3 GetPosition() const { return position; }
        glm::quat GetRotation() const { return rotation; }
        glm::vec3 GetScale() const { return scale; }

        glm::vec3 GetForward() const;
        glm::vec3 GetRight() const;
        glm::vec3 GetUp() const;

        glm::mat4 AsMatrix() const;
        void FromMatrix(glm::mat4 matrix);

        void SetEulerAngles(f32 yaw, f32 pitch, f32 roll);
        void GetEulerAngles(f32* yaw, f32* pitch, f32* roll) const;

        void LookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);
        bool FromJson(const json& j);

        bool RenderInMenu();
        bool RenderGuizmo();

        void Load(const json& j);

    };

} // Sogas
