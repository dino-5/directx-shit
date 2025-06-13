#pragma once
#include "EngineGfx/RenderPass.h"
#include <span>

enum PassResources
{
    RTXPass_ConstantBufferData,
    RTXPass_OutputTexture, // numFrames
    PassResourcesCount
};

void initRenderPassResources(GfxContext& context);
Resource*& getResource(GfxContext& context, u8 index);
std::span<Resource*> getPassResources(GfxContext& context, u8 index);

void forwardPassInit(GfxContext& context,
                     RenderPass& pass);
void forwardPassExecute(GfxContext& context,
                        Model* model,
                        RenderPass& pass,
                        void* data);
struct ForwardPassData
{
    bool drawModel;
};

void debugDrawBVHPassInit(GfxContext& context,
                     RenderPass& pass);
void debugDrawBVHPassExecute(GfxContext& context,
                        Model* model,
                        RenderPass& pass,
                        void* data);

void computeRTXPassInit(GfxContext& context,
                     RenderPass& pass);
void computeRTXPassExecute(GfxContext& context,
                        Model* model,
                        RenderPass& pass,
                        void* data);

struct RTXPassData
{
    u32 sphereCount;
    u32 imWidth;
    u32 imHeight;
    u32 dummy; // TODO :  why?? 
    Vector3 color;
};

void copyResourcePassExecute(GfxContext& context,
                        Model* model,
                        RenderPass& pass,
                        void* data);
struct CopyPassData
{
    Resource* presentedImage;
};
