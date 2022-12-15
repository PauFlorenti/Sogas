#include "camera_component.h"
#include "transform_component.h"

namespace Sogas
{

    DECL_OBJ_MANAGER("camera", TCompCamera);

    void TCompCamera::Load(const json& j)
    {
        mFovDeg = j.value("fov", mFovDeg);
        m_zMin  = j.value("near", m_zMin);
        m_zMax  = j.value("far", m_zMax);
        setProjectionParams(mFovDeg, mAspectRatio, m_zMin, m_zMax);
    }

    void TCompCamera::Update(const f32 /*dt*/)
    {
        TCompTransform* transform = Get<TCompTransform>();
        SASSERT(transform);
        glm::mat4 t = transform->AsMatrix();
        const glm::vec3 forward = glm::normalize(static_cast<glm::vec3>(t[2]));
        lookAt(transform->GetPosition(), transform->GetPosition() + forward);
    }

} // Sogas
