#pragma once

namespace Sogas
{
    glm::vec3 YawToVector(f32 yaw);
    /** Returns yaw in radians.*/
    f32 VectorToYaw(glm::vec3 front);
    glm::vec3 YawPitchToVector(f32 yaw, f32 pitch);
    /** Return yaw and pitch in radians.*/
    void VectorToYawPitch(glm::vec3 front, f32* yaw, f32* pitch);
}