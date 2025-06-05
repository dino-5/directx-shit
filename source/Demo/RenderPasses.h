#pragma once
#include "EngineGfx/RenderPass.h"

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
