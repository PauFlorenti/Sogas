
#include "texture.h"
#include "device_texture.h"
#include "render_device.h"

namespace Sogas
{
namespace Renderer
{
Texture::Texture(TextureDescriptor InDescriptor, void* data)
    : descriptor(InDescriptor)
{
}

void Texture::Destroy()
{
    delete internalState;
    internalState = nullptr;
}

const u32 Texture::GetWidth() const
{
    return descriptor.width;
}

const u32 Texture::GetHeight() const
{
    return descriptor.height;
}

const TextureDescriptor& Texture::GetDescriptor() const
{
    return descriptor;
}

void Texture::SetData(void* data) const
{
    internalState->SetData(data);
};
} // namespace Renderer
} // namespace Sogas