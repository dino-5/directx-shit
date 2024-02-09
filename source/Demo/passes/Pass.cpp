#include "Demo/passes/Pass.h"
#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/RootSignature.h"
#include "EngineGfx/RenderContext.h"

using namespace engine;

void Pass::SetPSO(std::string name)
{
	m_pso = graphics::PSO::GetPSO(name);
}

void Pass::SetRootSignature(std::string name)
{
	m_rootSignature = graphics::RootSignature::GetRootSignature(name);
}

void Pass::SetPipeline(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootSignature( * m_rootSignature);
	cmdList->SetPipelineState( * m_pso);
}
