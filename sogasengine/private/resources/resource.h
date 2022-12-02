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
    public:
        IResource(){};
        ~IResource(){};

        const std::string GetNameFile() const { return Name; }
        const std::string GetFullPath() const { return FullPath; }

        void SetResourceType(const IResourceType* ResourceType) { Type = std::move(ResourceType); }
        void SetResourceName(const std::string& ResourceName) { Name = std::move(ResourceName); }

        virtual void Destroy() {};

        const IResourceType* GetType() const { return Type; };

        template < typename Type >
        const Type* As()
        {
            assert( Type != nullptr );
            Type* ResourceType = GetResourceType<Type>();
            assert( ResourceType == Type );
            return static_cast<ResourceType>(this);
        }

    protected:
        std::string Name; // filename
        std::string FullPath; // Full path of the file

        const IResourceType* Type = nullptr;
    };

    class CResourceManager
    {
    public:
        CResourceManager* Get() {
            if(!Manager)
            {
                Manager = new CResourceManager();
            }
            return Manager;
        }

        bool Exists(const std::string& name)
        {
            return Resources.find(name) == Resources.end();
        }

        const IResource* GetResource(const std::string& name);

        void RegisterResourceType(IResourceType* NewResourceType)
        {
            assert(NewResourceType);

            for(u32 i = 0; i < NewResourceType->GetNumResourceTypeExtensions(); ++i)
            {
                const char* extension = NewResourceType->GetExtension(i);

                assert(ResourcesType.find(extension) != ResourcesType.end());
                ResourcesType[extension] = NewResourceType;
            }
        }

        void RegisterResource(IResource* Resource, const std::string& Name, const IResourceType* Type)
        {
            assert(Resource);
            assert(!Name.empty());
            assert(Type);

            Resource->SetResourceName(Name);
            Resource->SetResourceType(Type);

            // Validate resource is not already registered.
            assert(Resources.find(Name) == Resources.end());
            Resources[Name] = Resource;
        }

    private:
        static CResourceManager* Manager;

        std::map<std::string, IResource*> Resources;
        std::map<std::string, IResourceType*> ResourcesType;
    };

} // Sogas
