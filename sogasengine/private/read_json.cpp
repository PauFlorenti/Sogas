#include "read_json.h"

namespace Sogas
{
    glm::vec2 LoadVec2(const std::string& str)
    {
        glm::vec2 v;
        int n = sscanf_s(str.c_str(), "%f %f", &v.x, &v.y);
        if(n == 2)
            return v;
        SFATAL("Invalid reading string for a VEC2. Reading only %d values, 2 expected.", n);
        return glm::vec2(1.0f);
    }

    glm::vec2 LoadVec2(const json& j, const char* attr)
    {
        SASSERT(j.is_object())
        if(j.count(attr))
        {
            const std::string& str = j.value(attr, "");
            return LoadVec2(str);
        }
        return glm::vec2(0.0f);
    }

    glm::vec3 LoadVec3(const std::string& str)
    {
        glm::vec3 v;
        int n = sscanf_s(str.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
        if(n == 3)
            return v;
        SFATAL("Invalid str for a VEC3. Reading only %d values, 3 expected.", n)
        return glm::vec3(0.0f);
    }

    glm::vec3 LoadVec3(const json& j, const char* attr)
    {
        SASSERT(j.is_object());
        if(j.count(attr))
        {
            const std::string& str = j.value(attr, "");
            return LoadVec3(str);
        }
        return glm::vec3(0.0f);
    }

    glm::vec4 LoadVec4(const std::string& str)
    {
        glm::vec4 v;
        int n = sscanf_s(str.c_str(), "%f %f %f %f", &v.x, &v.y, &v.z, &v.w);
        if(n == 4)
            return v;
        SFATAL("Invalid str for a VEC3. Reading only %d values. Expected 4.", n)
        return glm::vec4(0.0f);
    }
    glm::vec4 LoadVec4(const json& j, const char* attr)
    {
        SASSERT(j.is_object());
        if(j.count(attr))
        {
            const std::string& str = j.value(attr, "");
            return LoadVec4(str);
        }
        return glm::vec4(0.0f);
    }

    glm::quat LoadQuat(const json& j, const char* attr)
    {
        SASSERT(j.is_object())
        if(j.count(attr))
        {
            const std::string str = j.value(attr, "");
            glm::quat q;
            int n = sscanf_s(str.c_str(), "%f %f %f %f", &q.x, &q.y, &q.z, &q.w);
            if(n == 4)
                return q;
            SFATAL("Invalid json reading QUAT attr %s. Only %d values red, 4 expected.", attr, n);
        }
        return glm::quat();
    }

    glm::vec4 LoadColor(const json& j)
    {
        glm::vec4 c;
        const auto& str = j.get<std::string>();
        int n = sscanf_s(str.c_str(), "%f %f %f %f", &c.x, &c.y, &c.z, &c.w);
        if(n == 4)
            return c;
        SFATAL("Invalid string reading for Color %s. %d value read, expected 4.", str.c_str(), n);
        return glm::vec4(1.0f);
    }

    glm::vec4 LoadColor(const json& j, const char* attr)
    {
        SASSERT(j.is_object())
        if(j.count(attr))
            return LoadColor(j[attr]);
        return glm::vec4(1.0f);
    }

    glm::vec4 LoadColor(const json& j, const char* attr, const glm::vec4& defaultValue)
    {
        if(j.count(attr) <= 0)
            return defaultValue;
        return LoadColor(j, attr);
    }

    json LoadJson(const std::string& filename)
    {
        json j;
        std::ifstream ifs(filename.c_str());
        SASSERT_MSG(ifs.is_open(), "Failed to open json file '%s'.", filename.c_str());

        try
        {
            j = json::parse(ifs);
        }
        catch(const std::exception& e)
        {
            ifs.close();
            std::cerr << e.what() << "\n";
            throw std::runtime_error("Failed to parse json file.");
        }
        return j;
    }

} // Sogas
