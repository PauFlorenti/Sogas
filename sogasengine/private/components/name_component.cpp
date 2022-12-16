#include "name_component.h"

namespace Sogas
{
    DECL_OBJ_MANAGER("name", CompName);
    std::unordered_map<std::string, CHandle> CompName::allNames;

    void CompName::Load(const json& j)
    {
        SASSERT(j.is_string());
        setName(j.get<std::string>().c_str());
    }

    void CompName::setName(const char* newName)
    {
        strcpy_s(name, newName);
        allNames[name] = CHandle(this);
    }

    CHandle getEntityByName(const std::string& name)
    {
        auto it = CompName::allNames.find(name);
        if (it == CompName::allNames.end())
            return CHandle();

        CHandle h = it->second;
        return h.GetOwner();
    }

} // Sogas
