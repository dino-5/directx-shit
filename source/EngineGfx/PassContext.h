#pragma once
#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/RootSignature.h"

namespace engine::graphics
{
	class PassContext
	{
	public:
		PassContext() = default;
		void Initialize();
		
	public:
		PSO* m_pso=nullptr;
		RootSignature* m_signature=nullptr;
	};
}