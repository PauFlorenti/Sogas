#pragma once

#include "render_types.h"
#include "resource.h"

namespace Sogas
{
namespace Renderer
{
class Texture;
}
class Material : public IResource
{
    const Renderer::Texture* albedo             = nullptr;
    const Renderer::Texture* normal             = nullptr;
    const Renderer::Texture* metallic_roughness = nullptr;
    const Renderer::Texture* emissive           = nullptr;

  public:
    bool CreateFromJson(const json& j);

    void Activate(Renderer::CommandBuffer cmd) const;
};
} // namespace Sogas
