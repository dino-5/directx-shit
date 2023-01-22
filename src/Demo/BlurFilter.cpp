//***************************************************************************************
// BlurFilter.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "include/BlurFilter.h"
 
BlurFilter::BlurFilter(UINT width, UINT height,
                       DXGI_FORMAT format)
{
	Init(width, height, format);
}

void BlurFilter::Init(UINT width, UINT height,
	DXGI_FORMAT format)
{
	mWidth = width;
	mHeight = height;
	mFormat = format;
	BuildResources();
}

ID3D12Resource* BlurFilter::Output()
{
	return m_blurMap0;
}

void BlurFilter::OnResize(UINT newWidth, UINT newHeight)
{
	if((mWidth != newWidth) || (mHeight != newHeight))
	{
		mWidth = newWidth;
		mHeight = newHeight;

		BuildResources();
	}
}
 
void BlurFilter::Execute(ComPtr<ID3D12GraphicsCommandList> cmdList, 
	                     ID3D12RootSignature* rootSig,
	                     ID3D12PipelineState* horzBlurPSO,
	                     ID3D12PipelineState* vertBlurPSO,
                         ID3D12Resource* input, 
						 int blurCount)
{
	auto weights = CalcGaussWeights(2.5f);
	int blurRadius = (int)weights.size() / 2;

	cmdList->SetComputeRootSignature(rootSig);

	cmdList->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
	cmdList->SetComputeRoot32BitConstants(0, (UINT)weights.size(), weights.data(), 1);

	auto resourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(input,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
	cmdList->ResourceBarrier(1, &resourceTransition);

	m_blurMap0.Transition(cmdList, ResourceState::COPY_DEST);

	cmdList->CopyResource(m_blurMap0, input);
	
	m_blurMap0.Transition(cmdList, ResourceState::GENERIC_READ_STATE);
	m_blurMap1.Transition(cmdList, ResourceState::UNORDERED_ACCESS);
 
	for(int i = 0; i < blurCount; ++i)
	{
		cmdList->SetPipelineState(horzBlurPSO);
		cmdList->SetComputeRootDescriptorTable(1, m_blurMap0.srv);
		cmdList->SetComputeRootDescriptorTable(2, m_blurMap1.uav);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
		cmdList->Dispatch(numGroupsX, mHeight, 1);

		m_blurMap0.Transition(cmdList, ResourceState::UNORDERED_ACCESS);
		m_blurMap1.Transition(cmdList, ResourceState::GENERIC_READ_STATE);

		// Vertical Blur pass.
		cmdList->SetPipelineState(vertBlurPSO);

		cmdList->SetComputeRootDescriptorTable(1, m_blurMap1.srv);
		cmdList->SetComputeRootDescriptorTable(2, m_blurMap0.uav);

		// How many groups do we need to dispatch to cover a column of pixels, where each
		// group covers 256 pixels  (the 256 is defined in the ComputeShader).
		UINT numGroupsY = (UINT)ceilf(mHeight / 256.0f);
		cmdList->Dispatch(mWidth, numGroupsY, 1);

		m_blurMap0.Transition(cmdList, ResourceState::GENERIC_READ_STATE);
		m_blurMap1.Transition(cmdList, ResourceState::UNORDERED_ACCESS);
	}

	resourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(input,
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->ResourceBarrier(1, &resourceTransition);

	cmdList->CopyResource(input, Output());
	

	resourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(input,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdList->ResourceBarrier(1, &resourceTransition);
}
 
std::vector<float> BlurFilter::CalcGaussWeights(float sigma)
{
	float twoSigma2 = 2.0f*sigma*sigma;

	// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
	// For example, for sigma = 3, the width of the bell curve is 
	int blurRadius = (int)ceil(2.0f * sigma);

	assert(blurRadius <= MaxBlurRadius);

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);
	
	float weightSum = 0.0f;

	for(int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;

		weights[i+blurRadius] = expf(-x*x / twoSigma2);

		weightSum += weights[i+blurRadius];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for(int i = 0; i < weights.size(); ++i)
	{
		weights[i] /= weightSum;
	}

	return weights;
}

void BlurFilter::BuildResources()
{
	m_blurMap0 = Resource(mFormat, mWidth, mHeight, 1, D3D12_RESOURCE_DIMENSION_TEXTURE2D, ResourceFlags::UNOURDERED, ResourceState::COMMON,
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), ResourceDescriptorFlags::UnorderedAccess | ResourceDescriptorFlags::ShaderResource);
	m_blurMap1 = Resource(mFormat, mWidth, mHeight, 1, D3D12_RESOURCE_DIMENSION_TEXTURE2D, ResourceFlags::UNOURDERED, ResourceState::COMMON,
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), ResourceDescriptorFlags::UnorderedAccess | ResourceDescriptorFlags::ShaderResource);
}