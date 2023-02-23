#pragma once
#include "Framework/graphics/dx12/PSO.h"
#include "Framework/graphics/dx12/RootSignature.h"
#include "Framework/graphics/RenderContext.h"

using namespace engine;

class SimplePass
{
public:
	SimplePass(engine::graphics::RenderContext& renderContext);
private:
	graphics::PSO* m_pso;
	graphics::RootSignature* m_rootSignature;
	
};

