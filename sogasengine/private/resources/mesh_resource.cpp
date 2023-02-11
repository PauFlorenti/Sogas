#include "resources/mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

template<> struct std::hash<Sogas::Vertex>
{
    size_t operator()(Sogas::Vertex const& vertex) const 
    {
        return (((std::hash<glm::vec3>()(vertex.position) ^
            (std::hash<glm::vec4>()(vertex.color) << 1)) >> 1) ^
            (std::hash<glm::vec2>()(vertex.uvs) << 1) >> 1) ^
            (std::hash<glm::vec3>()(vertex.normal) << 1);
    }
};

namespace Sogas
{
    class CMeshResource : public IResourceType
    {
        bool LoadMesh( CMesh* mesh, std::string filename ) const
        {
            auto name = CEngine::FindFile(std::move(filename));
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

                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    if (index.normal_index >= 0)
                    {
                        vertex.normal = {
                            attrib.normals[3 * index.normal_index + 0],
                            attrib.normals[3 * index.normal_index + 1],
                            attrib.normals[3 * index.normal_index + 2]
                        };
                    }

                    if (index.texcoord_index >= 0)
                    {
                        vertex.uvs = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            attrib.texcoords[2 * index.texcoord_index + 1]
                        };
                    }

                    vertex.color = glm::vec4(1.0f);

                    if (uniqueVertices.count(vertex) == 0)
                    {
                        uniqueVertices[vertex] = static_cast<u32>(vertices.size());
                        vertices.push_back(vertex);
                    }

                    indices.push_back(uniqueVertices[vertex]);
                }
            }

            // Update data to renderer
            bool ok = mesh->Create(vertices, indices, PrimitiveTopology::TRIANGLELIST);
            
            if (ok == false) {
                SERROR("Failed to create mesh.");
                return false;
            }

            return true;
        }

    public:
        const char* GetExtension( const i32 /*i*/ ) const override
        {
            return ".obj";
        }

        const char* GetName() const override { return "Mesh"; }
        IResource* Create( std::string name ) const override
        {
            CMesh* mesh = new CMesh();
            if(LoadMesh(mesh, std::move(name)))
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
