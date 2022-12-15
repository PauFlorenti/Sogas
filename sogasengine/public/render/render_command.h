#pragma once

namespace Sogas
{
    enum RenderCommandType
    {
        CreateTexture = 0,
        DestroyTexture = 1,
        CreateRenderable = 2,
        DestroyRenderable = 3,
        Count
    };

    class RenderCommand
    {
    public:
        const RenderCommandType GetType() const { return type; }

        template < typename paramType >
        const paramType GetParameters() const {
            return reinterpret_cast<paramType>(data);
        }

    private:
        RenderCommandType type;
        void* data;
    };

} // Sogas
