#include "resources/resource.h"

namespace Sogas
{
    CResourceManager* CResourceManager::ResourceManager = nullptr;

    const IResource* CResourceManager::GetResource( const std::string& name )
    {
        // Validate resource is not already registered.
        auto ResourceIt = Resources.find(name);

        // If registered, return resource.
        if( ResourceIt != Resources.end() )
            return ResourceIt->second;

        // If not registered, we have to load the resource.
        // Validate name has extension.
        size_t extensionIndex = name.find_last_of(".");

        if( extensionIndex == std::string::npos )
        {
            throw std::runtime_error("Could not find a valid extension.");
        }

        // Validate we can load this extension.
        std::string extension = name.substr( extensionIndex );
        auto type = ResourcesType.find( extension );

        if( type == ResourcesType.end() )
        {
            throw std::runtime_error("No registered extension.");
        }

        IResourceType* resourceType = type->second;
        SASSERT(resourceType);

        // Load resource.
        IResource* newResource = resourceType->Create( name );

        if( newResource == nullptr )
        {
            throw std::runtime_error("Failed to create the given resource");
        }

        // Register resource.
        RegisterResource(newResource, name, resourceType);

        return newResource;
    }
}