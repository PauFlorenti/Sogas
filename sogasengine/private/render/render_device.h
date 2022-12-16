#pragma once

#include "render_types.h"

// !TEMP should be removed
#include "components/camera_component.h"

namespace Sogas 
{
    class CMesh;

    class GPU_device
    {
    public:

        static std::shared_ptr<GPU_device> create(GraphicsAPI api, void* device);

        virtual ~GPU_device() {};
        
        virtual GraphicsAPI getApiType() const = 0;

        // TODO create general capabilities struct
        //virtual capabilities getCapabilities() const = 0;
        // TODO create general statistics struct
        // virtual statistics getStatistics() const = 0;

        virtual bool init() = 0;
        virtual void shutdown() = 0;
        virtual bool beginFrame() = 0;
        virtual void submitRenderCommands() = 0;
        virtual void endFrame() = 0;

        // ! TEMPORAL
        virtual void bind(const CMesh* mesh) = 0;
        virtual void draw(const CMesh* mesh) = 0;
        virtual void activateObject(const glm::mat4& model, const glm::vec4& color) = 0;
        virtual void activateCamera(const TCompCamera* camera) = 0;
        virtual bool CreateMesh(CMesh* mesh, const std::vector<Vertex>& vertices, PrimitiveTopology topology) = 0;
        virtual bool CreateMesh(CMesh* mesh, const std::vector<Vertex>& vertices, const std::vector<u32>& indices, PrimitiveTopology topology) = 0;
        
    protected:
        GraphicsAPI api_type;

    private:
        // Buffer createBufferWithSize()
    };
} // Sogas
