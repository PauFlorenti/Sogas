#include "engine.h"
#include "render/module_render.h"
#include "render_device.h"
#include "render_types.h"
#include "resources/resource.h"
#include "resources/texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace Sogas
{
using namespace Renderer;

class TextureResource : public IResourceType
{
    const u32   extensionNumber = 2;
    std::string extensions[2]   = {".png", ".text"};

    Texture* LoadTexture(std::string InName) const
    {
        Renderer::GPU_device* render = CEngine::Get()->GetRenderModule()->GetGraphicsDevice().get();
        SASSERT(render);

        size_t      extensionIndex = InName.find_last_of(".");
        std::string extension      = InName.substr(extensionIndex);

        Texture* texture = new Texture();

        TextureDescriptor desc;
        if (extension == extensions[0])
        {
            const auto filename = std::move(CEngine::FindFile(InName));
            i32        width, height, channels;
            stbi_uc*   pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            SASSERT(pixels);

            desc.type   = TextureDescriptor::TextureType::TEXTURE_TYPE_2D;
            desc.width  = static_cast<i16>(width);
            desc.height = static_cast<i16>(height);
            desc.format = Format::R8G8B8A8_SRGB;

            texture->descriptor = desc;
            texture->handle = render->CreateTexture(std::move(desc));
        }
        else if (extension == extensions[1])
        {
            desc.type   = TextureDescriptor::TextureType::TEXTURE_TYPE_2D;
            desc.width  = 1;
            desc.height = 1;
            desc.format = Format::R8G8B8A8_SRGB;

            texture->descriptor = desc;

            if (InName == "white.text")
            {
                u32 data = 0xFFFFFFFF;
                desc.data = (void*)&data;
            }
            else if (InName == "black.text")
            {
                desc.data = 0x00000000;
            }

            texture->handle = render->CreateTexture(std::move(desc));
        }

        return texture;
    }

  public:
    const char* GetExtension(const i32 i) const override
    {
        return extensions[i].c_str();
    }

    u32 GetNumResourceTypeExtensions() override
    {
        return extensionNumber;
    }

    const char* GetName() const override
    {
        return "Texture";
    }

    IResource* Create(std::string InName) const override
    {
        return LoadTexture(std::move(InName));
    }
};

void Texture::Destroy()
{
    auto renderer = CEngine::Get()->GetRenderModule()->GetGraphicsDevice();

    SASSERT_MSG(renderer != nullptr, "No renderer device while resources still alive.");

    renderer->DestroyTexture(handle);
    handle = INVALID_TEXTURE;
}

template <>
IResourceType* GetResourceType<Texture>()
{
    static TextureResource factory;
    return &factory;
}
} // namespace Sogas
