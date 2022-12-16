#pragma once

#include "base_component.h"
#include "entity/entity.h"

namespace Sogas
{
    class CompName : public TCompBase
    {
        DECL_SIBILING_ACCESS();

        char name[64];

    public:
        static std::unordered_map<std::string, CHandle> allNames;

        void Load(const json& j);
        void setName(const char* newName);
        const char* getName() const { return name; }
    };
} // Sogas
