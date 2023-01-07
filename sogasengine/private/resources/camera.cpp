#include "resources/camera.h"

namespace Sogas
{
    void CCamera::updateViewProjection()
    {
        view_projection = view * projection;
    }

    void CCamera::lookAt(const glm::vec3 eye, const glm::vec3 target, const glm::vec3 up)
    {
        mEye    = eye;
        mTarget = target;
        view    = glm::lookAtRH(eye, target, up);
        updateViewProjection();

        mFront  = glm::normalize(target - eye);
        mRight  = glm::normalize(glm::cross(mFront, -up));
        mUp     = glm::cross(mFront, mRight);
    }

    void CCamera::setProjectionParams(const f32 fovDeg, const f32 aspectRatio, const f32 near, const f32 far)
    {
        mFovDeg = fovDeg;
        mAspectRatio = aspectRatio;
        m_zMin = near;
        m_zMax = far;
        projection = glm::perspective(glm::radians(mFovDeg), mAspectRatio, m_zMin, m_zMax);
        updateViewProjection();

        bIsOrthogonal = false;
    }

} // Sogas
