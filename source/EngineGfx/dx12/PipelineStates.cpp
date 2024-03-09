#include "PipelineStates.h"
#include "EngineGfx/dx12/d3dx12.h"

D3D12_COMPARISON_FUNC CastType(ComparisonFunc obj)
{
	return static_cast<D3D12_COMPARISON_FUNC>(obj);
}

D3D12_DEPTH_WRITE_MASK CastType(DepthWriteMask obj)
{
	return static_cast<D3D12_DEPTH_WRITE_MASK>(obj);
}

D3D12_STENCIL_OP CastType(StencilOP obj)
{
	return static_cast<D3D12_STENCIL_OP>(obj);
}

BlendState::BlendState(ColorBlendEquation cl_eq, AlphaBlendEquation al_eq, WriteEnable wr)
{
	m_desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	m_desc.RenderTarget[0].BlendEnable    = true;
	m_desc.RenderTarget[0].LogicOpEnable  = false;
	m_desc.RenderTarget[0].BlendOp        = static_cast<D3D12_BLEND_OP>(cl_eq.op);
	m_desc.RenderTarget[0].SrcBlend       = static_cast<D3D12_BLEND>(cl_eq.srcFactor);
	m_desc.RenderTarget[0].DestBlend      = static_cast<D3D12_BLEND>(cl_eq.dstFactor);
    m_desc.RenderTarget[0].SrcBlendAlpha  = D3D12_BLEND_ONE;
    m_desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	m_desc.RenderTarget[0].BlendOpAlpha   = D3D12_BLEND_OP_ADD;
    m_desc.RenderTarget[0].LogicOp        = D3D12_LOGIC_OP_NOOP;
    m_desc.RenderTarget[0].RenderTargetWriteMask = static_cast<D3D12_COLOR_WRITE_ENABLE>(wr);
}

BlendState::BlendState(WriteEnable wr)
{
	m_desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    m_desc.RenderTarget[0].RenderTargetWriteMask = static_cast<D3D12_COLOR_WRITE_ENABLE>(wr);
}

DepthStencilState::DepthStencilState()
{
	m_desc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
}

void DepthStencilState::SetDepthState(DepthState state)
{
    m_desc.DepthEnable      = state.isEnabled;
    m_desc.DepthWriteMask   = CastType(state.writeMask);
    m_desc.DepthFunc        = CastType(state.depthFunc);
}
void DepthStencilState::SetStencilState(StencilState state)
{
    m_desc.StencilEnable    = state.isEnabled;
    m_desc.StencilReadMask  = state.readMask;
    m_desc.StencilWriteMask = state.writeMask;
	
	m_desc.FrontFace = state.frontFace;
	m_desc.BackFace  = state.backFace;
}

DepthStencilState::DepthStencilState(DepthState depthState, StencilState stencilState)
{
	SetDepthState(depthState);
	SetStencilState(stencilState);
}

RasterizerState::RasterizerState(CullMode cull, bool frontCounterClockwise)
{
	m_desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	m_desc.CullMode = static_cast<D3D12_CULL_MODE>(cull);
	m_desc.FrontCounterClockwise = frontCounterClockwise;
}
