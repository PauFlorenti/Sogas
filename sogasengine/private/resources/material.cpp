#include "resources/material.h"
#include "engine.h"
#include "render/module_render.h"
#include "render/render_command.h"
#include "render_device.h"
#include "resources/texture.h"

namespace Sogas
{
class Material_resource : public IResourceType
{
    bool LoadMaterial(Material* InMaterial, std::string InName) const
    {
        json j = LoadJson(std::move(CEngine::FindFile(InName)));
        InMaterial->CreateFromJson(j);
        return true;
    }

  public:
    const char* GetExtension(const i32 /*i*/) const override
    {
        return ".mat";
    }
    const char* GetName() const override
    {
        return "Material";
    }
    IResource* Create(std::string InName) const override
    {
        Material* material = new Material();
        if (LoadMaterial(material, std::move(InName)))
            return material;
        return nullptr;
    }
};

bool Material::CreateFromJson(const json& j)
{
    std::string albedo_name = j.value("albedo", "");
    albedo                  = albedo_name.empty() ? CResourceManager::Get()->GetResource("white.text")->As<Texture>() : CResourceManager::Get()->GetResource(albedo_name)->As<Texture>();

    std::string normal_name = j.value("normal", "");
    normal                  = normal_name.empty() ? CResourceManager::Get()->GetResource("white.text")->As<Texture>() : CResourceManager::Get()->GetResource(normal_name)->As<Texture>();

    std::string metallic_roughness_name = j.value("metallic_roughness", "");
    metallic_roughness                  = metallic_roughness_name.empty() ? CResourceManager::Get()->GetResource("white.text")->As<Texture>() : CResourceManager::Get()->GetResource(metallic_roughness_name)->As<Texture>();

    std::string emissive_name = j.value("emissive", "");
    emissive                  = emissive_name.empty() ? CResourceManager::Get()->GetResource("white.text")->As<Texture>() : CResourceManager::Get()->GetResource(emissive_name)->As<Texture>();
    return true;
}

void Material::Destroy()
{
    albedo             = nullptr;
    normal             = nullptr;
    metallic_roughness = nullptr;
    emissive           = nullptr;
}

// void Material::Activate(Renderer::CommandBuffer /*cmd*/) const
// {
//     auto renderer = CEngine::Get()->GetRenderModule()->GetGraphicsDevice();
//     // renderer->BindTexture(albedo, cmd.activePipeline, 0, 1);
//     // renderer->BindTexture(normal, cmd.activePipeline, 1, 1);
//     // renderer->BindTexture(metallic_roughness, cmd.activePipeline, 2, 1);
//     // renderer->BindTexture(emissive, cmd.activePipeline, 3, 1);
// }

template <>
IResourceType* GetResourceType<Material>()
{
    static Material_resource factory;
    return &factory;
}
} // namespace Sogas
