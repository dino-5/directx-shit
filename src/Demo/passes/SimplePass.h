#pragma once
#include "Framework/graphics/dx12/PSO.h"
#include "Framework/graphics/dx12/RootSignature.h"


class SimplePass
{
public:
	SimplePass();
private:
	PSO m_pso;
	RootSignature m_rootSignature;
};

