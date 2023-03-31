#pragma once

#include "render_types.h"
#include "resource.h"

namespace Sogas
{
class Texture : public IResource
{
  public:
    void Destroy() override;
    
    Renderer::TextureHandle     handle = Renderer::INVALID_TEXTURE;
    Renderer::TextureDescriptor descriptor;
};
} // namespace Sogas