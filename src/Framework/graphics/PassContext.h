#pragma once
#include "Framework/graphics/dx12/PSO.h"
#include "Framework/graphics/dx12/RootSignature.h"

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