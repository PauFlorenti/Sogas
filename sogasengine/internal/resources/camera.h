#pragma once 

namespace Sogas
{
    class CCamera
    {
    protected:
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 view_projection;

        glm::vec3 mEye;
        glm::vec3 mTarget;
        
        glm::vec3 mFront;
        glm::vec3 mRight;
        glm::vec3 mUp;

        f32 m_zMin = 0.01f;
        f32 m_zMax = 1000.0f;
        f32 mFovDeg = 45.0f;
        f32 mAspectRatio = 1.0f;

        bool bIsOrthogonal;

    public:
        const glm::mat4 GetView() const { return view; }
        const glm::mat4 GetProjection() const { return projection; }
        const glm::mat4 GetViewProjection() const { return view_projection; }

        void updateViewProjection();
        void lookAt(const glm::vec3 eye, const glm::vec3 target, const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));
        void setProjectionParams(const f32 fovDeg, const f32 aspectRatio, const f32 near, const f32 far);
        // TODO setOrthogonalParams
    };

} // Sogas
