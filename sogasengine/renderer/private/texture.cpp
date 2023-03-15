
#include "texture.h"
#include "device_texture.h"
#include "render_device.h"

namespace Sogas
{
namespace Renderer
{

TextureDescriptor& TextureDescriptor::SetSize(u16 InWidth, u16 InHeight, u16 InDepth)
{
    width  = InWidth;
    height = InHeight;
    depth  = InDepth;
    return *this;
}
TextureDescriptor& TextureDescriptor::SetFlags(u8 InMipmaps, u8 InFlags)
{
    mipmaps = InMipmaps;
    flags   = InFlags;
    return *this;
}
TextureDescriptor& TextureDescriptor::SetFormatType(Format InFormat, TextureType InType)
{
    format      = InFormat;
    type = InType;
    return *this;
}
TextureDescriptor& TextureDescriptor::SetName(std::string InName)
{
    name = InName;
    return *this;
}
TextureDescriptor& TextureDescriptor::SetData(void* InData)
{
    data = InData;
    return *this;
}

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