#ifndef PIPELINE_STATES_H
#define PIPELINE_STATES_H

#include <d3d12.h>
#include <string>
#include <vector>
#include "../include/common.h"

class InputLayoutElement
{
public:
	enum class Format
	{
		float3 = DXGI_FORMAT_R32G32B32_FLOAT,
		float2 = DXGI_FORMAT_R32G32_FLOAT,
	};
	unsigned int GetFormatSize();
	unsigned int GetOffset()const { return m_offset; }
	InputLayoutElement(std::string name, Format format, unsigned int offset=0) : 
		m_name(name),
		m_format(format),
		m_offset(offset)
	{}
	InputLayoutElement() = default;

	operator D3D12_INPUT_ELEMENT_DESC() const
	{
		return { m_name.c_str(), 0, DXGI_FORMAT(static_cast<int>(m_format)), 0, GetOffset(),
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	}
public:
	std::string m_name;
	Format m_format;
	unsigned int m_offset;
};

class InputLayout
{
public:
	InputLayout() = default;
	InputLayout(std::vector<InputLayoutElement> layout) : m_layout(layout) {}
	void AddLayoutElement(InputLayoutElement el)
	{
		if (m_layout.size())
			el.m_offset = m_layout.back().GetOffset() + m_layout.back().GetFormatSize();
		else
			el.m_offset = 0;

		m_layout.push_back(el);
	}

	operator D3D12_INPUT_LAYOUT_DESC()
	{
		D3D12_INPUT_LAYOUT_DESC value;
		array.resize(m_layout.size());
		for (int i = 0; i < m_layout.size(); i++)
			array[i] = m_layout[i];

		value.pInputElementDescs = array.data();
		value.NumElements = m_layout.size();
		return value;
	}

public:
	std::vector<InputLayoutElement> m_layout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> array;
};

#undef TRANSPARENT
enum class BlendOP
{
	ADD,
	SUBTRACT,
	REV_SUBTRACT
};
enum class BlendColorFactor
{
	ZERO = D3D12_BLEND_ZERO	,
	ONE = D3D12_BLEND_ONE,
	SRC_COLOR = D3D12_BLEND_SRC_COLOR,
	INV_SRC_COLOR = D3D12_BLEND_INV_SRC_COLOR,
	DST_COLOR = D3D12_BLEND_DEST_COLOR,
	INV_DST_COLOR = D3D12_BLEND_INV_DEST_COLOR,
	SRC_ALPHA = D3D12_BLEND_SRC_ALPHA,
	INV_SRC_ALPHA = D3D12_BLEND_INV_SRC_ALPHA,
	DST_ALPHA = D3D12_BLEND_DEST_ALPHA,
	INV_DST_ALPHA = D3D12_BLEND_INV_DEST_ALPHA,
};

enum class BlendAlphaFactor
{
	ZERO = D3D12_BLEND_ZERO	,
	ONE = D3D12_BLEND_ONE,
	SRC_ALPHA = D3D12_BLEND_SRC_ALPHA,
	INV_SRC_ALPHA = D3D12_BLEND_INV_SRC_ALPHA,
	DST_ALPHA = D3D12_BLEND_DEST_ALPHA,
	INV_DST_ALPHA = D3D12_BLEND_INV_DEST_ALPHA,
};

template<typename Factor>
struct BlendEquation 
{
	Factor srcFactor;
	Factor dstFactor;
	BlendOP op;
};

using ColorBlendEquation = BlendEquation<BlendColorFactor>;
using AlphaBlendEquation = BlendEquation<BlendAlphaFactor>;

class BlendState
{
public:
	enum class CasualStates
	{
		DEFAULT,
		TRANSPARENT
	};
	BlendState(ColorBlendEquation cl_eq, AlphaBlendEquation al_eq);
	BlendState();
	operator D3D12_BLEND_DESC()const { return m_desc;  }

public:
	D3D12_BLEND_DESC m_desc;
};

enum class ComparisonFunc:int 
{
	NEVER   = D3D12_COMPARISON_FUNC_NEVER	,
	LESS    = D3D12_COMPARISON_FUNC_LESS,
	EQUAL   = D3D12_COMPARISON_FUNC_EQUAL,
	LE      = D3D12_COMPARISON_FUNC_LESS_EQUAL,
	GREATER = D3D12_COMPARISON_FUNC_GREATER,
	NE      = D3D12_COMPARISON_FUNC_NOT_EQUAL,
	GE      = D3D12_COMPARISON_FUNC_GREATER_EQUAL,
	ALWAYS  = D3D12_COMPARISON_FUNC_ALWAYS,
};

enum class DepthWriteMask:int
{
	ZERO = D3D12_DEPTH_WRITE_MASK_ZERO,
	ALL  = D3D12_DEPTH_WRITE_MASK_ALL	
};

enum class StencilOP
{
	KEEP     = D3D12_STENCIL_OP_KEEP,
	ZERO     = D3D12_STENCIL_OP_ZERO,
	REPLACE  = D3D12_STENCIL_OP_REPLACE,
	INCR_SAT = D3D12_STENCIL_OP_INCR_SAT,
	DECR_SAT = D3D12_STENCIL_OP_DECR_SAT,
	INVERT   = D3D12_STENCIL_OP_INVERT,
	INCR     = D3D12_STENCIL_OP_INCR,
	DECR     = D3D12_STENCIL_OP_DECR,
};

D3D12_COMPARISON_FUNC CastType(ComparisonFunc obj);

D3D12_DEPTH_WRITE_MASK CastType(DepthWriteMask obj);

D3D12_STENCIL_OP CastType(StencilOP obj);

struct DepthState
{
	bool isEnabled           = true;
	DepthWriteMask writeMask = DepthWriteMask::ALL;
	ComparisonFunc depthFunc = ComparisonFunc::LESS;
};

struct StencilOpDesc
{
	StencilOP failOP           = StencilOP::KEEP;
	StencilOP depthFailOP      = StencilOP::KEEP;
	StencilOP passOP           = StencilOP::KEEP;
	ComparisonFunc stencilFunc = ComparisonFunc::ALWAYS;
	operator D3D12_DEPTH_STENCILOP_DESC()
	{
		return { CastType(failOP), CastType(depthFailOP), CastType(passOP), CastType(stencilFunc) };
	}
};

struct StencilState
{
	bool isEnabled             = true;
	uint8 readMask             = 0xff;
	uint8 writeMask            = 0xff;
	StencilOpDesc frontFace;
	StencilOpDesc backFace;
};

class DepthStencilState
{
public:
	DepthStencilState();
	operator D3D12_DEPTH_STENCIL_DESC()
	{
		return m_desc;
	}
	DepthStencilState(DepthState, StencilState);
	void SetDepthState(DepthState state);
	void SetStencilState(StencilState state);

public:
	D3D12_DEPTH_STENCIL_DESC m_desc;
};



#endif