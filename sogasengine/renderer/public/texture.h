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

    R32G32B32A32_SFLOAT,
    R32G32B32A32_SINT,
    R32G32B32A32_UINT,

    R32G32B32_SFLOAT,
    R32G32B32_SINT,
    R32G32B32_UINT,

    R32G32_SFLOAT,
    R32G32_SINT,
    R32G32_UINT,

    R32_SFLOAT,
    R32_SINT,
    R32_UINT,

    R16G16B16A16_SFLOAT,
    R16G16B16A16_SINT,
    R16G16B16A16_UINT,
    R16G16B16A16_SSCALED,
    R16G16B16A16_USCALED,
    R16G16B16A16_SNORM,
    R16G16B16A16_UNORM,

    R16G16B16_SFLOAT,
    R16G16B16_SINT,
    R16G16B16_UINT,
    R16G16B16_SSCALED,
    R16G16B16_USCALED,
    R16G16B16_SNORM,
    R16G16B16_UNORM,

    R16G16_SFLOAT,
    R16G16_SINT,
    R16G16_UINT,
    R16G16_SSCALED,
    R16G16_USCALED,
    R16G16_SNORM,
    R16G16_UNORM,

    R16_SFLOAT,
    R16_SINT,
    R16_UINT,
    R16_SSCALED,
    R16_USCALED,
    R16_SNORM,
    R16_UNORM,

    R8G8B8A8_SRGB,
    R8G8B8A8_SINT,
    R8G8B8A8_UINT,
    R8G8B8A8_SSCALED,
    R8G8B8A8_USCALED,
    R8G8B8A8_SNORM,
    R8G8B8A8_UNORM,

    R8G8B8_SRGB,
    R8G8B8_SINT,
    R8G8B8_UINT,
    R8G8B8_SSCALED,
    R8G8B8_USCALED,
    R8G8B8_UNORM,
    R8G8B8_SNORM,

    R8G8_SRGB,
    R8G8_SINT,
    R8G8_UINT,
    R8G8_SSCALED,
    R8G8_USCALED,
    R8G8_UNORM,
    R8G8_SNORM,

    R8_SRGB,
    R8_SINT,
    R8_UINT,
    R8_SSCALED,
    R8_USCALED,
    R8_UNORM,
    R8_SNORM,

    D32_SFLOAT,
    S8_UINT,
    D16_UNORM_S8_UINT,
    D24_UNORM_S8_UINT,
    D32_UNORM_S8_UINT
};

enum class Usage
{
    DEFAULT  = 0, // no CPU access, GPU read/write
    UPLOAD   = 1, // CPU write, GPU read
    READBACK = 2  // CPU read, GPU write
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

class DeviceTexture;

struct TextureDescriptor
{
    enum TextureType
    {
        TEXTURE_TYPE_1D,
        TEXTURE_TYPE_2D,
        TEXTURE_TYPE_3D
    } textureType = TextureType::TEXTURE_TYPE_2D;

    u32       width     = 0;
    u32       height    = 0;
    u32       depth     = 0;
    Format    format    = Format::UNDEFINED;
    Usage     usage     = Usage::DEFAULT;
    BindPoint bindPoint = BindPoint::NONE;
};

class Texture final : public IResource
{
  public:
    Texture(TextureDescriptor descriptor);
    ~Texture() { Destroy(); };

    void Destroy() override;

    const u32 GetWidth() const;
    const u32 GetHeight() const;
    const TextureDescriptor& GetDescriptor() const;

    void SetData(void* data) const;

    DeviceTexture* internalState = nullptr;
  private:
    TextureDescriptor              descriptor;
};
} // namespace Renderer
} // namespace Sogas