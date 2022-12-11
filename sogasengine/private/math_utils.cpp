#include "math_utils.h"

namespace Sogas
{
    glm::vec3 YawToVector(f32 yaw)
    {
        return glm::vec3(sinf(yaw), 0.0f, cosf(yaw));
    }

    f32 VectorToYaw(glm::vec3 front)
    {
        return atan2f(front.x, front.z);
    }

    glm::vec3 YawPitchToVector(f32 yaw, f32 pitch)
    {
        return glm::vec3(sinf(yaw) * cosf(-pitch),
                                    sinf(-pitch),
                        cosf(yaw) * cosf(-pitch));
    }

    void VectorToYawPitch(glm::vec3 front, f32* yaw, f32* pitch)
    {
        *yaw = atan2f(front.x, front.z);
        f32 mdo = sqrtf(front.x * front.x + front.z * front.z);
        *pitch = atan2f(-front.y, mdo);
    }
} // Sogas
