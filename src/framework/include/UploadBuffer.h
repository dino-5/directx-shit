#pragma once

#include "Util.h"
#include "common.h"
#include "Framework/dx12/Device.h"

class UploadBuffer
{
public:
    UploadBuffer( uint elementCount, uint sizeOfType , bool isConstantBuffer)  
    {
        Init(elementCount, sizeOfType, isConstantBuffer);
    }

    void Init(uint elementCount, uint sizeOfType, bool isConstantBuffer) 
    {
        auto device = Device::GetDevice();
        mIsConstantBuffer = isConstantBuffer;
        mElementByteSize = sizeOfType;

        if(isConstantBuffer)
            mElementByteSize = d3dUtil::CalcConstantBufferByteSize(sizeOfType);

        auto heapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount); 
        ThrowIfFailed(device->CreateCommittedResource(
            &heapDesc,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&mUploadBuffer)));

        ThrowIfFailed(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));

    }

    UploadBuffer() = default;
    UploadBuffer(UploadBuffer&& rhs):
        mUploadBuffer(std::move(rhs.mUploadBuffer)),
        mMappedData(std::move(rhs.mMappedData)),
        mElementByteSize(std::move(rhs.mElementByteSize)),
        mIsConstantBuffer(std::move(rhs.mIsConstantBuffer))
    {}
    UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
    ~UploadBuffer()
    {
        if(mUploadBuffer != nullptr)
            mUploadBuffer->Unmap(0, nullptr);

        mMappedData = nullptr;
    }

    ID3D12Resource* Resource()const
    {
        return mUploadBuffer.Get();
    }

    void CopyData(int elementIndex, const void* data)
    {
        memcpy(&mMappedData[elementIndex*mElementByteSize], data, mElementByteSize);
    }

public:
    Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
    BYTE* mMappedData = nullptr;

    UINT mElementByteSize = 0;
    bool mIsConstantBuffer = false;
};