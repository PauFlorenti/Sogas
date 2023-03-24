#pragma once

// TODO FIX THIS
#include "../internal/resources/resource.h"

namespace Sogas
{
namespace Renderer
{

enum class Format
{
    UNDEFINED = 0,

    R8_UNORM,
    R8_SNORM,
    R8_USCALED,
    R8_SSCALED,
    R8_UINT,
    R8_SINT,
    R8_SRGB,

    R8G8_UNORM,
    R8G8_SNORM,
    R8G8_USCALED,
    R8G8_SSCALED,
    R8G8_UINT,
    R8G8_SINT,
    R8G8_SRGB,

    R8G8B8_UNORM,
    R8G8B8_SNORM,
    R8G8B8_USCALED,
    R8G8B8_SSCALED,
    R8G8B8_UINT,
    R8G8B8_SINT,
    R8G8B8_SRGB,

    B8G8R8_UNORM,
    B8G8R8_SNORM,
    B8G8R8_USCALED,
    B8G8R8_SSCALED,
    B8G8R8_UINT,
    B8G8R8_SINT,
    B8G8R8_SRGB,

    R8G8B8A8_UNORM,
    R8G8B8A8_SNORM,
    R8G8B8A8_USCALED,
    R8G8B8A8_SSCALED,
    R8G8B8A8_UINT,
    R8G8B8A8_SINT,
    R8G8B8A8_SRGB,

    B8G8R8A8_UNORM,
    B8G8R8A8_SNORM,
    B8G8R8A8_USCALED,
    B8G8R8A8_SSCALED,
    B8G8R8A8_UINT,
    B8G8R8A8_SINT,
    B8G8R8A8_SRGB,

    R16_UNORM,
    R16_SNORM,
    R16_USCALED,
    R16_SSCALED,
    R16_UINT,
    R16_SINT,
    R16_SFLOAT,

    R16G16_UNORM,
    R16G16_SNORM,
    R16G16_USCALED,
    R16G16_SSCALED,
    R16G16_UINT,
    R16G16_SINT,
    R16G16_SFLOAT,

    R16G16B16_UNORM,
    R16G16B16_SNORM,
    R16G16B16_USCALED,
    R16G16B16_SSCALED,
    R16G16B16_UINT,
    R16G16B16_SINT,
    R16G16B16_SFLOAT,

    R16G16B16A16_UNORM,
    R16G16B16A16_SNORM,
    R16G16B16A16_USCALED,
    R16G16B16A16_SSCALED,
    R16G16B16A16_UINT,
    R16G16B16A16_SINT,
    R16G16B16A16_SFLOAT,

    R32_UINT,
    R32_SINT,
    R32_SFLOAT,

    R32G32_UINT,
    R32G32_SINT,
    R32G32_SFLOAT,

    R32G32B32_UINT,
    R32G32B32_SINT,
    R32G32B32_SFLOAT,

    R32G32B32A32_UINT,
    R32G32B32A32_SINT,
    R32G32B32A32_SFLOAT,

    R64_UINT,
    R64_SINT,
    R64_SFLOAT,

    R64G64_UINT,
    R64G64_SINT,
    R64G64_SFLOAT,

    R64G64B64_UINT,
    R64G64B64_SINT,
    R64G64B64_SFLOAT,

    R64G64B64A64_UINT,
    R64G64B64A64_SINT,
    R64G64B64A64_SFLOAT,

    D16_UNORM,
    D32_SFLOAT,
    S8_UINT,
    D16_UNORM_S8_UINT,
    D24_UNORM_S8_UINT,
    D32_UNORM_S8_UINT
};

inline bool IsDepthStencil(Format InFormat)
{
    return InFormat >= Format::D16_UNORM_S8_UINT || InFormat == Format::D24_UNORM_S8_UINT || InFormat == Format::D32_UNORM_S8_UINT;
}

inline bool IsDepthOnly(Format InFormat)
{
    return InFormat == Format::D32_SFLOAT || InFormat == Format::D16_UNORM;
}

inline bool IsStencilOnly(Format InFormat)
{
    return InFormat == Format::S8_UINT;
}

inline bool HasDepth(Format InFormat)
{
    return InFormat >= Format::D16_UNORM_S8_UINT || InFormat == Format::D32_SFLOAT || InFormat == Format::D16_UNORM;
}

inline bool HasDepthOrStencil(Format InFormat)
{
    return InFormat >= Format::D16_UNORM && InFormat <= Format::D32_UNORM_S8_UINT;
}

enum class Usage
{
    DEFAULT  = 0, // no CPU access, GPU read/write
    UPLOAD   = 1, // CPU write, GPU read
    READBACK = 2 // CPU read, GPU write
};

enum class BindPoint
{
    NONE          = 0,
    VERTEX        = 1 << 0,
    INDEX         = 1 << 1,
    UNIFORM       = 1 << 2,
    RENDER_TARGET = 1 << 3,
    DEPTH_STENCIL = 1 << 4,
    SHADER_SAMPLE = 1 << 5
};

enum class TextureFlags
{
    DEFAULT,
    RENDER_TARGET,
    COMPUTE,
    COUNT
};

enum TextureFlagsMask
{
    DEFAULT       = 1 << 0,
    RENDER_TARGET = 1 << 1,
    COMPUTE       = 1 << 2
};

class DeviceTexture;
struct TextureDescriptor
{
    enum class TextureType
    {
        TEXTURE_TYPE_1D,
        TEXTURE_TYPE_2D,
        TEXTURE_TYPE_3D
    };

    void*       data = nullptr;

    u16 width   = 1;
    u16 height  = 1;
    u16 depth   = 1;
    u8  mipmaps = 1;
    u8  flags   = 0;

    Format      format    = Format::UNDEFINED;
    TextureType type      = TextureType::TEXTURE_TYPE_2D;
    std::string name;

    TextureDescriptor& SetSize(u16 InWidth, u16 InHeight, u16 InDepth);
    TextureDescriptor& SetFlags(u8 InMipmaps, u8 InFlags);
    TextureDescriptor& SetFormatType(Format InFormat, TextureType InType);
    TextureDescriptor& SetName(std::string InName);
    TextureDescriptor& SetData(void* InData);
};

class Texture final : public IResource
{
  public:
    Texture() = default;
    Texture(TextureDescriptor descriptor, void* data = nullptr);
    ~Texture()
    {
        Destroy();
    };

    void Destroy() override;

    const u32                GetWidth() const;
    const u32                GetHeight() const;
    const TextureDescriptor& GetDescriptor() const;

    void SetData(void* data) const;

    DeviceTexture* internalState = nullptr;

    TextureDescriptor descriptor;

  private:
};
} // namespace Renderer
} // namespace Sogas