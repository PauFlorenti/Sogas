#include "transform_component.h"

namespace Sogas
{
    DECL_OBJ_MANAGER("transform", TCompTransform);

    glm::vec3 TCompTransform::GetForward() const
    {
        return glm::normalize(AsMatrix()[2]);
    }

    glm::vec3 TCompTransform::GetRight() const
    {
        return glm::normalize(AsMatrix()[0]);
    }

    glm::vec3 TCompTransform::GetUp() const
    {
        return glm::normalize(AsMatrix()[1]);
    }

    glm::mat4 TCompTransform::AsMatrix() const
    {
        return glm::scale(glm::mat4(1), scale) 
            * glm::mat4_cast(rotation)
            * glm::translate(glm::mat4(1), position);
    }

    void TCompTransform::FromMatrix(glm::mat4 matrix)
    {
        glm::vec3 dummy_vec3;
        glm::vec4 dummy_vec4;
        glm::decompose(matrix, scale, rotation, position, dummy_vec3, dummy_vec4);
    }

    /** Set Euler angles in radians.*/
    void TCompTransform::SetEulerAngles(f32 yaw, f32 pitch, f32 roll)
    {
        rotation = glm::quat(glm::vec3(pitch, yaw, roll));
    }

    void TCompTransform::GetEulerAngles(f32* yaw, f32* pitch, f32* roll) const
    {
        glm::vec3 fwd = GetForward();
        VectorToYawPitch(fwd, yaw, pitch);

        if(!roll)
        {
            glm::vec3 rollZeroLeft = glm::cross(glm::vec3(0, 0, 1), fwd);
            glm::vec3 rollZeroUp = glm::cross(fwd, rollZeroLeft);
            glm::vec3 realLeft = -GetRight();
            realLeft = glm::normalize(realLeft);
            rollZeroLeft = glm::normalize(rollZeroLeft);
            rollZeroUp = glm::normalize(rollZeroUp);
            f32 rolledLeftOnUp = glm::dot(realLeft, rollZeroUp);
            f32 rolledLeftOnLeft = glm::dot(realLeft, rollZeroLeft);
            *roll = atan2f(rolledLeftOnUp, rolledLeftOnLeft);
        }
    }

    void TCompTransform::LookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& /*up*/)
    {
        position = eye;
        glm::vec3 fwd = target - eye;
        f32 yaw, pitch;
        VectorToYawPitch(fwd, &yaw, &pitch);
        SetEulerAngles(yaw, pitch, 0.f);
    }

    bool TCompTransform::FromJson(const json& j)
    {
        if(j.count("pos"))
        {
            position = LoadVec3(j, "pos");
        }
        if(j.count("lookat"))
        {
            LookAt(GetPosition(), LoadVec3(j, "lookAt"), glm::vec3(0, 1, 0));
        }
        if(j.count("rot"))
        {
            rotation = LoadQuat(j, "rot");
        }
        if(j.count("euler"))
        {
            glm::vec3 euler = LoadVec3(j, "euler");
            euler.x = glm::radians(euler.x);
            euler.y = glm::radians(euler.y);
            euler.z = glm::radians(euler.z);
            rotation = glm::quat(euler);
        }
        if(j.count("scale"))
        {
            const json& jscale = j["scale"];
            if(jscale.is_number())
            {
                f32 fscale = jscale.get<f32>();
                scale = glm::vec3(fscale);
            }
            else {
                scale = LoadVec3(j, "scale");
            }
        }
        return true;
    }

    bool TCompTransform::RenderInMenu()
    {
        return true;
    }

    bool TCompTransform::RenderGuizmo()
    {
        return true;
    }

    void TCompTransform::Load(const json& j)
    {
        FromJson(j);
    }

} // Sogas
