#pragma once 

namespace Sogas
{
    class CCamera
    {
    protected:
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 view_projection;

    public:
        const glm::mat4 GetView() const { return view; }
        const glm::mat4 GetProjection() const { return projection; }
        const glm::mat4 GetViewProjection() const { return view_projection; }
    };

} // Sogas
