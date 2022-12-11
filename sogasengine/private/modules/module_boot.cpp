#include "module_boot.h"
#include "entity/entity.h"

namespace Sogas
{

    static void PaserScene(const std::string filename)
    {
        //json j = LoadJson(filename);

        SASSERT(j.is_array());

        for(u32 i = 0; i < j.size(); ++i)
        {
            const json& jitem = j[i];

            if(jitem.count("entity"))
            {
                const json& jentity = jitem["entity"];

                CHandle entity_handle;

                entity_handle.Create<CEntity>();

                CEntity* entity = entity_handle;
                entity->Load(jentity);
            }
        }
    }

    bool CModuleBoot::Start()
    {
        json j = LoadJson("../../../sogasengine/private/boot.json");
        auto scenes = j["scenes_to_load"].get<std::vector<std::string>>();
        for(auto s : scenes)
        {
            LoadScene(s);
        }
        return true;
    }

    void CModuleBoot::LoadScene(const std::string& filename)
    {
        STRACE("Parsing scene '%s'.", filename.c_str());
        PaserScene(filename);
    }
} // Sogas
