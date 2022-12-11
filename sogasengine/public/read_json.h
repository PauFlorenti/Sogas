#pragma once

namespace Sogas
{
    glm::vec2 LoadVec2(const std::string& str);
    glm::vec2 LoadVec2(const json& j, const char* attr);
    glm::vec3 LoadVec3(const std::string& str);
    glm::vec3 LoadVec3(const json& j, const char* attr);
    glm::vec4 LoadVec4(const std::string& str);
    glm::vec4 LoadVec4(const json& j, const char* attr);
    glm::quat LoadQuat(const json& j, const char* attr);

    glm::vec4 LoadColor(const json& j);
    glm::vec4 LoadColor(const json& j, const char* attr);
    glm::vec4 LoadColor(const json& j, const char* attr, const glm::vec4 defaultValue);

    json LoadJson(const std::string& filename);

} // Sogas
