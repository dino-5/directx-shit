#include "Demo/passes/Pass.h"
#include "Framework/graphics/dx12/PSO.h"
#include "Framework/graphics/dx12/RootSignature.h"
#include "Framework/graphics/RenderContext.h"

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
