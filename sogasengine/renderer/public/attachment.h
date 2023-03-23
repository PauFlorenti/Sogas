#pragma once

#include "texture.h"
namespace Sogas
{
namespace Renderer
{

// struct Attachment
// {
//     enum class Type
//     {
//         RENDERTARGET,
//         DEPTH_STENCIL
//     } type = Type::RENDERTARGET;

//     const Texture*               texture               = nullptr;
//     const AttachmentFramebuffer* attachmentFramebuffer = nullptr;
//     BindPoint                    initialLayout         = BindPoint::NONE;
//     BindPoint                    subpassLayout         = BindPoint::NONE;
//     BindPoint                    finalLayout           = BindPoint::NONE;
//     f32                          clear[4]; // Clear attachment value.

//     enum class LoadOp
//     {
//         LOAD,
//         CLEAR,
//         DONTCARE
//     } loadop = LoadOp::LOAD;

//     enum class StoreOp
//     {
//         STORE,
//         DONTCARE
//     } storeop = StoreOp::STORE;

//     static Attachment RenderTarget(const Texture*               texture               = nullptr,
//                                    const AttachmentFramebuffer* attachmentFramebuffer = nullptr,
//                                    LoadOp                       loadop                = LoadOp::CLEAR,
//                                    StoreOp                      storeop               = StoreOp::STORE,
//                                    BindPoint                    initialLayout         = BindPoint::NONE,
//                                    BindPoint                    subpassLayout         = BindPoint::RENDER_TARGET,
//                                    BindPoint                    finalLayout           = BindPoint::SHADER_SAMPLE)
//     {
//         Attachment att;
//         att.type                  = Attachment::Type::RENDERTARGET;
//         att.texture               = texture;
//         att.attachmentFramebuffer = attachmentFramebuffer;
//         att.loadop                = loadop;
//         att.storeop               = storeop;
//         att.initialLayout         = initialLayout;
//         att.subpassLayout         = subpassLayout;
//         att.finalLayout           = finalLayout;
//         return att;
//     }

//     static Attachment DepthStencil(const Texture*               texture               = nullptr,
//                                    const AttachmentFramebuffer* attachmentFramebuffer = nullptr,
//                                    LoadOp                       loadop                = LoadOp::CLEAR,
//                                    StoreOp                      storeop               = StoreOp::DONTCARE,
//                                    BindPoint                    initialLayout         = BindPoint::DEPTH_STENCIL,
//                                    BindPoint                    subpassLayout         = BindPoint::DEPTH_STENCIL,
//                                    BindPoint                    finalLayout           = BindPoint::DEPTH_STENCIL)
//     {
//         Attachment att;
//         att.type                  = Attachment::Type::DEPTH_STENCIL;
//         att.attachmentFramebuffer = attachmentFramebuffer;
//         att.texture               = texture;
//         att.loadop                = loadop;
//         att.storeop               = storeop;
//         att.initialLayout         = initialLayout;
//         att.subpassLayout         = subpassLayout;
//         att.finalLayout           = finalLayout;
//         return att;
//     }
// };
} // namespace Renderer
} // namespace Sogas