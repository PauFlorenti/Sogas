
#include "texture.h"
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

} // namespace Renderer
} // namespace Sogas