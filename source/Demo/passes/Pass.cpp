#include "Demo/passes/Pass.h"
#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/RootSignature.h"
#include "EngineGfx/RenderContext.h"

using namespace engine;

void Pass::setPSO(std::wstring name)
{
	m_pso = graphics::PSO::GetPSO(name);
}

void Pass::setRootSignature(graphics::RootSignatureType type)
{
	m_rootSignature = graphics::RootSignature::GetRootSignature(type);
}

void Pass::setPipeline(ID3D12GraphicsCommandList* cmdList)
{
}
