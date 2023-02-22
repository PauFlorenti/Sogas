#include "texture.h"

namespace Sogas
{
namespace Renderer
{
Texture::Texture(TextureDescriptor InDescriptor)
    : descriptor(InDescriptor)
{}

void Texture::Destroy()
{
    delete internalState;
    internalState = nullptr;
    //internalState.release();
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

void Texture::SetData() const {};
} // namespace Renderer
} // namespace Sogas