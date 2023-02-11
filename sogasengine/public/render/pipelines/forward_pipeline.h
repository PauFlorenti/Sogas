#pragma once

#include "renderer/public/render_types.h"

namespace Sogas
{
    struct Swapchain;
    namespace Renderer
    {
        class GPU_device;
        class Buffer;

        class ForwardPipeline
        {
        public:
            explicit ForwardPipeline(std::shared_ptr<GPU_device> InRenderer = nullptr, std::shared_ptr<Swapchain> swapchain = nullptr);
            ~ForwardPipeline(){};

            void update_constants();
            void render(std::shared_ptr<Swapchain> swapchain);
            void present();
            void destroy();

        private:
            std::shared_ptr<GPU_device> renderer;
            Pipeline pipeline;
            RenderPass forwardRenderPass;
            Shader forwardShaders[2]; // 0 vs, 1 ps
            Pipeline presentPipeline;
            Shader presentShaders[2];

            AttachmentFramebuffer colorAttachment;
            AttachmentFramebuffer depthAttachment;

            std::shared_ptr<Buffer> constantBuffer;
            std::shared_ptr<Buffer> lightBuffer;
            std::shared_ptr<Buffer> quadIdxBuffer;
            std::shared_ptr<Buffer> quadBuffer;

            std::shared_ptr<Texture> colorBuffer;

            const u32 nLights = 10;
        };
    }
}