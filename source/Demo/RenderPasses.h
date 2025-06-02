#pragma once
#include "EngineGfx/RenderPass.h"

void forwardPassInit(GfxContext* context,
                     RenderPass& pass);
void forwardPassExecute(GfxContext* context,
                        const Model& model,
                        const RenderPass& pass);
