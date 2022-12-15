#pragma once

namespace Sogas
{
    class IResource;

    class IResourceType
    {
    public:
        virtual bool Open(const std::string& filename) = 0;
        virtual void Create() = 0;
        virtual const char* GetExtension(const i32 i) const = 0;
        virtual u32 GetNumResourceTypeExtensions() { return 1; }
        virtual const char* GetName() const = 0;
        virtual IResource* Create(const std::string& name) const = 0;
    };

    template< typename T >
    IResourceType* GetResourceType();

    class IResource
    {
    protected:
        std::string Name; // filename
        std::string FullPath; // Full path of the file
        const IResourceType* Type = nullptr;

    public:
        ~IResource(){};

        const std::string GetNameFile() const { return Name; }
        const std::string GetFullPath() const { return FullPath; }

        void SetResourceType(const IResourceType* ResourceType) { Type = std::move(ResourceType); }
        void SetResourceName(const std::string& ResourceName) { Name = std::move(ResourceName); }

        virtual void Destroy() {};

        const IResourceType* GetType() const { return Type; };

        template < typename TargetType >
        const TargetType* As() const
        {
            SASSERT( Type != nullptr );
            const IResourceType* target_type = GetResourceType<TargetType>();
            SASSERT( target_type == Type );
            return static_cast<const TargetType*>(this);
        }
    };

    class CResourceManager
    {
    public:
        static CResourceManager* Get() {
            if(!ResourceManager)
            {
                ResourceManager = new CResourceManager();
            }
            return ResourceManager;
        }

        bool Exists(const std::string& name)
        {
            return Resources.find(name) == Resources.end();
        }

        const IResource* GetResource(const std::string& name);

        void RegisterResourceType(IResourceType* NewResourceType)
        {
            SASSERT(NewResourceType);

            for(u32 i = 0; i < NewResourceType->GetNumResourceTypeExtensions(); ++i)
            {
                const char* extension = NewResourceType->GetExtension(i);

                SASSERT((ResourcesType.find(extension) == ResourcesType.end()));
                ResourcesType[extension] = NewResourceType;
            }
        }

        void RegisterResource(IResource* Resource, const std::string& Name, const IResourceType* Type)
        {
            SASSERT(Resource);
            SASSERT(!Name.empty());
            SASSERT(Type);

            Resource->SetResourceName(Name);
            Resource->SetResourceType(Type);

            // Validate resource is not already registered.
            SASSERT( Resources.find(Name) == Resources.end() );
            Resources[Name] = Resource;
        }

    private:
        static CResourceManager* ResourceManager;

        std::map<std::string, IResource*> Resources;
        std::map<std::string, IResourceType*> ResourcesType;
    };

} // Sogas
