#pragma once
#include <d3d12.h>
#include "EngineCommon/include/common.h"
#include "EngineCommon/include/defines.h"

namespace engine::graphics
{

	struct CommandQueueDesc

	{
		D3D12_COMMAND_LIST_TYPE  type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	};

	class CommandQueue
	{
	public:
		CommandQueue() = default;
		CommandQueue(ID3D12Device* device, CommandQueueDesc desc)
		{
			Init(device, desc);
		}

		void Init(ID3D12Device* device, CommandQueueDesc desc)
		{
			D3D12_COMMAND_QUEUE_DESC descQ{
				.Type = desc.type,
				.Priority = 0,
				.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
				.NodeMask = 0
			};
			device->CreateCommandQueue(&descQ, IID_PPV_ARGS(&m_queue));
		}

		ID3D12CommandQueue* operator->() { return m_queue; }

		SHIT_ENGINE_GET_D3D12COMPONENT(ID3D12CommandQueue, Queue, m_queue);
		SHIT_ENGINE_NON_COPYABLE(CommandQueue);

		void Reset() { m_queue->Release(); }

	private:
		ID3D12CommandQueue* m_queue = nullptr;
	};
};

