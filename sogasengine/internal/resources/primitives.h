#pragma once

namespace Sogas
{
    bool RegisterPrimitives();

    void ReleasePrimitives();

    void DrawLine(glm::vec3 src, glm::vec3 dest, glm::vec4 color);
    //void DrawWiredAABB();
    //void DrawWiredSphere(const glm::mat4 world, f32 radius, glm::vec4 color);
} // Sogas
