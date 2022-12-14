#include "mesh.h"
#include "resource.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

template<> struct std::hash<Sogas::Vertex>
{
    size_t operator()(Sogas::Vertex const& vertex) const 
    {
        return (std::hash<glm::vec3>()(vertex.Position) ^
            (std::hash<glm::vec4>()(vertex.Color) << 1));
    }
};

namespace Sogas
{
    const std::vector<Vertex> vs = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
    };

    const std::vector<u32> is = {
        0, 1, 2, 2, 3, 0
    };

    class CMeshResource : public IResourceType
    {
        bool LoadMesh( CMesh* mesh, const std::string& name ) const
        {
            // Load data
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;

            std::string warn, err;

            bool ret = tinyobj::LoadObj(&attrib, &shapes, nullptr, &warn, &err, name.c_str());

            if(!warn.empty())
                SWARNING("'%s'", warn.c_str());

            if(!err.empty())
                SERROR("'%s'", err.c_str());

            if(!ret){
                SFATAL("Failed to load .obj '%s'.", name.c_str());
                return false;
            }

            std::vector<Vertex> vertices;
            std::vector<u32> indices;

            std::unordered_map<Vertex, u32> uniqueVertices{};

            for (const auto& shape : shapes)
            {
                for (const auto& index : shape.mesh.indices)
                {
                    Vertex vertex = {};

                    vertex.Position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    if (uniqueVertices.count(vertex) == 0)
                    {
                        uniqueVertices[vertex] = static_cast<u32>(vertices.size());
                        vertices.push_back(vertex);
                    }

                    indices.push_back(uniqueVertices[vertex]);
                }
            }

            // Update data to renderer
            bool ok = false;
            if (indices.size() > 0)
            {
                ok = mesh->Create(vs, is, PrimitiveTopology::TRIANGLELIST);
            }
            else
            {
                ok = mesh->Create(vertices, PrimitiveTopology::TRIANGLELIST);
            }

            if (ok == false) {
                SERROR("Failed to create mesh.");
                return false;
            }

            return true;
        }

    public:
        bool Open( const std::string& /*filename*/ ) override { return true; };
        void Create() override {};
        const char* GetExtension( const i32 /*i*/ ) const override
        {
            return ".obj";
        }


        const char* GetName() const override { return "Mesh"; }
        IResource* Create( const std::string& name ) const override
        {
            CMesh* mesh = new CMesh();
            if(LoadMesh(mesh, name))
                return mesh;
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
