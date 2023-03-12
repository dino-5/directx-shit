#pragma once

#include "Framework/util/Util.h"
#include "Framework/graphics/dx12/DescriptorHeap.h"
#include "Framework/graphics/dx12/PSO.h"
#include "Framework/graphics/dx12/Resource.h"
#include "Framework/graphics/dx12/Device.h"

class BlurFilter
{
public:
	BlurFilter(UINT width, UINT height,
		DXGI_FORMAT format);
	BlurFilter() = default;
	void Init(UINT width, UINT height,
		DXGI_FORMAT format);
		
	BlurFilter(const BlurFilter& rhs)=delete;
	BlurFilter& operator=(const BlurFilter& rhs)=delete;
	~BlurFilter()=default;

	ID3D12Resource* Output();


	void OnResize(UINT newWidth, UINT newHeight);

	void Execute(
		ID3D12GraphicsCommandList* cmdList, 
		ID3D12RootSignature* rootSig,
		ID3D12PipelineState* horzBlurPSO,
		ID3D12PipelineState* vertBlurPSO,
		ID3D12Resource* input, 		
		int blurCount);

private:
	std::vector<float> CalcGaussWeights(float sigma);

	void BuildResources();

private:

	const int MaxBlurRadius = 5;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	ViewID blur0Srv;
	ViewID blur0Uav;

	ViewID blur1Srv;
	ViewID blur1Uav;

	Resource m_blurMap0;
	Resource m_blurMap1;
};
