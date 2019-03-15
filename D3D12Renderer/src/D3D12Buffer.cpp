#include "hzpch.h"
#include "D3D12Buffer.h"

#include "D3D12Command.h"
#include "D3D12Renderer.h"
#include "D3D12RenderPass.h"

namespace Hazel {

	void D3D12Buffer::Build(uint32_t size, const void* data, const CreateInfo& ci)
	{
		auto device = static_cast<D3D12Renderer*>(m_Renderer)->m_Device;
		m_Usage = ci.Usage;
		m_Size = size;
		m_VertexStride = ci.VertexStride;
		m_IsDynamic = ci.Dynamic;

		if (ci.Usage & eTransferDst)
		{
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(size),
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_Buffer)
			);

			auto stagingBuffer = m_Renderer->CreateBuffer(size, data, Buffer::CreateInfo::GetStaging());
			auto srcBuffer = static_cast<D3D12Buffer*>(stagingBuffer.get())->m_Buffer;

			// Create a new command list?
			ID3D12CommandAllocator* allocator;
			device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&allocator)
			);

			ID3D12GraphicsCommandList* cmdList;
			device->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				allocator,
				nullptr,
				IID_PPV_ARGS(&cmdList)
			);

			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = data;
			subresourceData.RowPitch = size;
			subresourceData.SlicePitch = subresourceData.RowPitch;

			UpdateSubresources(cmdList, m_Buffer, srcBuffer, 0, 0, 1, &subresourceData);

			cmdList->ResourceBarrier(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					m_Buffer,
					D3D12_RESOURCE_STATE_COPY_DEST,
					D3D12_RESOURCE_STATE_GENERIC_READ
				)
			);

			// Execute command list?
			cmdList->Close();
			ID3D12CommandList* cmdLists[] = { cmdList };
			auto cmdQueue = static_cast<D3D12Renderer*>(m_Renderer)->m_CommandQueue;
			cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
			
			ID3D12Fence* fence;
			uint64_t fenceValue = 0U;
			device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
			
			fenceValue++;
			cmdQueue->Signal(fence, fenceValue);

			if (fence->GetCompletedValue() < fenceValue)
			{
				HANDLE event = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
				fence->SetEventOnCompletion(fenceValue, event);
				WaitForSingleObject(event, INFINITE);
				CloseHandle(event);
			}
		}
		else if (data)
		{
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(size),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_Buffer)
			);

			void* mapped = D3D12Buffer::Map(size, 0);
			memcpy(mapped, data, size);
			D3D12Buffer::Unmap();
		}
	}

	void* D3D12Buffer::Map(uint32_t size, uint32_t offset)
	{
		m_Buffer->Map(0, nullptr, reinterpret_cast<void**>(&m_Mapped));
		m_IsMapped = true;
		return m_Mapped;
	}

	void D3D12Buffer::Unmap()
	{
		m_Buffer->Unmap(0, nullptr);
		m_IsMapped = false;
	}

	void D3D12Buffer::Flush()
	{
	}

	void D3D12Buffer::Bind(Command* cmd)
	{
		auto cmdList = static_cast<D3D12Command*>(cmd)->m_CommandList;

		if (m_Usage & Buffer::eVertex)
		{
			D3D12_VERTEX_BUFFER_VIEW view;
			view.BufferLocation = m_Buffer->GetGPUVirtualAddress();
			view.StrideInBytes = m_VertexStride;
			view.SizeInBytes = m_Size;
			cmdList->IASetVertexBuffers(0, 1, &view);
		}

		if (m_Usage & Buffer::eIndex)
		{
			D3D12_INDEX_BUFFER_VIEW view;
			view.BufferLocation = m_Buffer->GetGPUVirtualAddress();
			view.Format = DXGI_FORMAT_R16_UINT;  /* TODO: Change this! (Put on CreateInfo) */
			view.SizeInBytes = m_Size;
			cmdList->IASetIndexBuffer(&view);
		}
	}

	void D3D12Buffer::Cleanup()
	{
		m_Buffer->Release();
	}

}