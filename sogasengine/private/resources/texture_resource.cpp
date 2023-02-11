#include "engine.h"
#include "resources/resource.h"
#include "render_device.h"
#include "render_types.h"
#include "render/module_render.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace Sogas
{
    class TextureResource : public IResourceType
    {
        const u32 extensionNumber = 2;
        std::string extensions[2] = {".png", ".text"};

        bool LoadTexture(Texture* InTexture, std::string InName) const
        {

            Renderer::GPU_device* render = CEngine::Get()->GetRenderModule()->GetGraphicsDevice().get();
            SASSERT(render);

            size_t extensionIndex = InName.find_last_of(".");
            std::string extension = InName.substr( extensionIndex );

            TextureDescriptor desc;
            if (extension == extensions[0])
            {
                const auto filename = std::move(CEngine::FindFile(InName));
                i32 width, height, channels;
                stbi_uc* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
                SASSERT(pixels);

                desc.textureType    = TextureDescriptor::TEXTURE_TYPE_2D;
                desc.width          = width;
                desc.height         = height;
                desc.format         = Format::R8G8B8A8_SRGB;
                desc.usage          = Usage::UPLOAD;
                desc.bindPoint      = BindPoint::SHADER_SAMPLE;
                render->CreateTexture(&desc, pixels, InTexture);
            }
            else if (extension == extensions[1])
            {
                desc.textureType    = TextureDescriptor::TEXTURE_TYPE_2D;
                desc.width          = 1;
                desc.height         = 1;
                desc.format         = Format::R8G8B8A8_SRGB;
                desc.usage          = Usage::UPLOAD;
                desc.bindPoint      = BindPoint::SHADER_SAMPLE;

                if (InName == "white.text")
                {
                    u32 data = 0xFFFFFFFF;
                    render->CreateTexture(&desc, static_cast<void*>(&data), InTexture);
                }
                else if (InName == "black.text")
                {
                    render->CreateTexture(&desc, 0x00000000, InTexture);
                }
            }
            else
            {
                SERROR("Something went bad when loading a texture ...");
                return false;
            }

            return true;
        }

    public:
        const char* GetExtension( const i32 i) const override
        {
            return extensions[i].c_str();
        }

        u32 GetNumResourceTypeExtensions() override { return extensionNumber; }

        const char* GetName() const override { return "Texture"; }

        IResource* Create( std::string InName ) const override
        {
            Texture* texture = new Texture();
            if (LoadTexture(texture, std::move(InName)))
            {
                return texture;
            }
            return nullptr;
        }
    };

    template<>
    IResourceType* GetResourceType<Texture>()
    {
        static TextureResource factory;
        return &factory;
    }
} // Sogas
