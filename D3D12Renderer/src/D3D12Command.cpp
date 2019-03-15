#include "hzpch.h"
#include "D3D12Command.h"

#include "D3D12Renderer.h"
#include "D3D12RenderPass.h"

namespace Hazel {

	void D3D12Command::Build()
	{	
		auto device = static_cast<D3D12Renderer*>(m_Parent->GetRenderer())->m_Device;
		if (FAILED(device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			__uuidof(ID3D12CommandAllocator), (void**)&m_Allocator)))
		{
			//HZ_ERROR("Failed to create D3D12 command allocator")
			return;
		}
	}

	void D3D12Command::Viewport(float x, float y, float width, float height, float minDepth, float maxDepth)
	{
		auto cmdList = static_cast<D3D12RenderPass*>(m_Parent)->m_CommandList;
		cmdList->RSSetViewports(
			1,
			{ &CD3DX12_VIEWPORT(x, y, width, height, minDepth, maxDepth) }
		);
	}

	void D3D12Command::Scissor(int x, int y, int width, int height)
	{
		auto cmdList = static_cast<D3D12RenderPass*>(m_Parent)->m_CommandList;
		cmdList->RSSetScissorRects(
			1,
			{ &(D3D12_RECT() = { x, y, width, height }) }
		);
	}

	void D3D12Command::Draw(
		uint32_t vertexCount,
		uint32_t instanceCount,
		uint32_t firstVertex,
		uint32_t firstInstance)
	{
		m_CommandList->DrawInstanced(
			vertexCount,
			instanceCount,
			firstVertex,
			firstInstance
		);
	}

	void D3D12Command::DrawIndexed(
		uint32_t indexCount,
		uint32_t instanceCount,
		uint32_t firstIndex,
		int32_t vertexOffset, uint32_t firstInstance)
	{
		m_CommandList->DrawIndexedInstanced(
			indexCount,
			instanceCount,
			firstIndex,
			vertexOffset, 
			firstInstance
		);
	}

	void D3D12Command::Cleanup()
	{
	}

	void D3D12Command::SetCommandListFromParent()
	{
		m_CommandList = static_cast<D3D12RenderPass*>(m_Parent)->m_CommandList;
	}

}