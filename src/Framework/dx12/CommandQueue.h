#pragma once
#include <d3d12.h>
#include "Framework/include/common.h"
#include "Framework/include/defines.h"

struct CommandQueueDesc

{
	D3D12_COMMAND_LIST_TYPE  type = D3D12_COMMAND_LIST_TYPE_DIRECT;
};

class CommandQueue
{
public:
	CommandQueue(ID3D12Device* device, CommandQueueDesc desc)
	{

		D3D12_COMMAND_QUEUE_DESC descQ{
			.Type = desc.type,
			.Priority = 0,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			.NodeMask = 0
		};
		device->CreateCommandQueue(&descQ, IID_PPV_ARGS(&m_queue));
	}

	SHIT_ENGINE_GET_D3D12COMPONENT(ID3D12CommandQueue, Queue, m_queue);

private:
	ComPtr<ID3D12CommandQueue> m_queue;
};
