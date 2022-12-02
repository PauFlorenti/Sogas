#include "mesh.h"
#include "resource.h"

namespace Sogas
{
    class CMeshResource : public IResourceType
    {
    public:
        bool Open( const std::string& /*filename*/ ) override { return true; };
        void Create() override {};
        const char* GetExtension( const i32 /*i*/ ) const override
        {
            return ".obj";
        }

        bool LoadMesh( CMesh* /*mesh*/, const std::string& /*name*/ )
        {
            // Load data

            // Update data to renderer

            // Initiate a unique mesh
        }

        const char* GetName() const override { return "Mesh"; }
        IResource* Create( const std::string& /*name*/ ) const override
        {
            // Call load data
            return nullptr;
        }

    };

    template<>
    IResourceType* GetResourceType<CMesh>()
    {
        static CMeshResource factory;
        return &factory;
    }

} // Sogas
