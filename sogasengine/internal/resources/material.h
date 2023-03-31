#pragma once

#include "render_types.h"
#include "resource.h"

namespace Sogas
{
class Texture;
class Material : public IResource
{
    const Texture* albedo             = nullptr;
    const Texture* normal             = nullptr;
    const Texture* metallic_roughness = nullptr;
    const Texture* emissive           = nullptr;

  public:
    bool CreateFromJson(const json& j);

    void Destroy() override;

    //void Activate(Renderer::CommandBuffer cmd) const;
};
} // namespace Sogas
