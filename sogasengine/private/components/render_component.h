#pragma once

#include "base_component.h"

namespace Sogas
{
    class CMesh;
    class Material;

    class TCompRender : public TCompBase
    {
        struct DrawCall
        {
            const CMesh*    mesh = nullptr;
            bool            enabled = true;
            const Material* material = nullptr;

            bool Load(const json& j);
        };

        void RemoveFromRenderManager();

    public:

        ~TCompRender();
        void Load(const json& j);
        void RenderDebug();
        void OnEntityCreated();

        void UpdateRenderManager();

        std::vector<DrawCall> DrawCalls;
    };

} // Sogas
