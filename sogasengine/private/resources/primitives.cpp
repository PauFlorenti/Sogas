#include "primitives.h"
#include "mesh.h"

namespace Sogas
{
    static CMesh* line = nullptr;
    static CMesh* unit_wired_cube = nullptr;
    static CMesh* unit_wired_sphere = nullptr;

    static void CreateLine(CMesh& mesh)
    {
        std::vector<Vertex> vertices = {
            {glm::vec3(0.0f), glm::vec4(1.0f)},
            {glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f)}
        };

        mesh.Create(vertices, PrimitiveTopology::LINELIST);
    }

    bool RegisterPrimitives()
    {
        {
            CMesh* mesh = new CMesh();
            CreateLine(*mesh);
            line = mesh;
        }
        {

        }
        return true;
    }

    void DrawLine(glm::vec3 src, glm::vec3 dst, glm::vec4 /*color*/)
    {
        glm::vec3 delta = dst - src;
        f32 length = static_cast<f32>(delta.length());
        if(length < 1e-4f)
            return;
        
        //glm::mat4 world = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, -length)) * glm::inverse(glm::lookAt(src, dst, glm::vec3(0, 1, 0))); 
        line->Activate();
        line->Render();
    }

/*
    void DrawWiredAABB()
    {

    }

    void DrawWiredSphere(const glm::mat4 world, f32 radius, glm::vec4 color)
    {

    }
*/

} // Sogas
