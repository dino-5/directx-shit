#ifndef PIPELINE_STATES_H
#define PIPELINE_STATES_H

#include <d3d12.h>
#include <string>
#include <vector>
#include "EngineCommon/include/types.h"
#include "EngineCommon/include/common.h"

#undef TRANSPARENT
enum class BlendOP
{
	ADD=1,
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

enum class WriteEnable : uint
{
        RED = D3D12_COLOR_WRITE_ENABLE_RED	,
        GREEN = D3D12_COLOR_WRITE_ENABLE_GREEN	,
        BLUE = D3D12_COLOR_WRITE_ENABLE_BLUE,
        ALPHA = D3D12_COLOR_WRITE_ENABLE_ALPHA,
        ALL = D3D12_COLOR_WRITE_ENABLE_ALL,
		NONE = 0
};

template<typename Factor>
struct BlendEquation 
{
	Factor srcFactor = Factor::ONE;
	Factor dstFactor = Factor::ZERO;
	BlendOP op = BlendOP::ADD;
	BlendEquation(Factor src= Factor::ONE, Factor dst =Factor::ZERO, BlendOP o = BlendOP::ADD) : srcFactor(src), dstFactor(dst), op(o) {}
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
	BlendState(ColorBlendEquation cl_eq, AlphaBlendEquation al_eq,
		WriteEnable wr = WriteEnable::ALL);
	BlendState(WriteEnable wr = WriteEnable::ALL);
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
	DepthState(bool Enabled, DepthWriteMask Mask, ComparisonFunc Func): isEnabled(Enabled), writeMask(Mask), depthFunc(Func)
	{}
	DepthState()=default;
};

struct StencilOpDesc
{
	StencilOP failOP           ;
	StencilOP depthFailOP      ;
	StencilOP passOP           ;
	ComparisonFunc stencilFunc ;
	StencilOpDesc(StencilOP fail= StencilOP::KEEP, StencilOP depthFail= StencilOP::KEEP, StencilOP pass= StencilOP::KEEP,
		ComparisonFunc func = ComparisonFunc::ALWAYS) 
		:failOP(fail), depthFailOP(depthFail), passOP(pass), stencilFunc(func)
	{}
	operator D3D12_DEPTH_STENCILOP_DESC()
	{
		return { CastType(failOP), CastType(depthFailOP), CastType(passOP), CastType(stencilFunc) };
	}
};

struct StencilState
{
	bool isEnabled             = true;
	u8 readMask             = 0xff;
	u8 writeMask            = 0xff;
	StencilOpDesc frontFace = StencilOpDesc();
	StencilOpDesc backFace  = StencilOpDesc();
	StencilState(bool Enabled = true, u8 read = 0xff, u8 write = 0xff,
		StencilOpDesc front = StencilOpDesc(), StencilOpDesc back = StencilOpDesc()) : isEnabled(Enabled), readMask(read), writeMask(write),
		frontFace(front), backFace(back) {}
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

enum class CullMode
{
	NONE  = D3D12_CULL_MODE_NONE,
	FRONT = D3D12_CULL_MODE_FRONT,
	BACK  =D3D12_CULL_MODE_BACK
};

class RasterizerState
{
public:
	RasterizerState(CullMode cull = CullMode::BACK, bool frontCounterClockwise = false);
	operator D3D12_RASTERIZER_DESC()
	{
		return m_desc;
	}

public:
	D3D12_RASTERIZER_DESC m_desc;
};



#endif