#pragma once

#include "Hazel/Rendering/Command.h"

// Forward declarations:
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;

namespace Hazel {

	class HAZEL_API D3D12Command : public Command
	{
	public:
		friend class D3D12Buffer;
		friend class D3D12RenderPass;

		//! CTOR/DTOR:
		D3D12Command(RenderPass* parent);
		virtual ~D3D12Command();

		//! VIRTUALS:
		void Build() override;
		void Viewport(float x, float y, float width, float height, float minDepth, float maxDepth) override;
		void Scissor(int x, int y, int width, int height) override;

		void Draw(
			uint32_t vertexCount,
			uint32_t instanceCount,
			uint32_t firstVertex,
			uint32_t firstInstance
		) override;

		void DrawIndexed(
			uint32_t indexCount,
			uint32_t instanceCount,
			uint32_t firstIndex,
			int32_t vertexOffset,
			uint32_t firstInstance
		) override;

		void Cleanup() override;

		//! SERVICES:
		void SetCommandListFromParent();

	protected:
		//! MEMBERS:
		ID3D12CommandAllocator* m_Allocator;
		ID3D12GraphicsCommandList* m_CommandList;
	};

	////////////////////////////////////////////////////////////////////////////////
	// D3D12Command inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline D3D12Command::D3D12Command(RenderPass* parent) :
		Command(parent),
		m_Allocator(nullptr),
		m_CommandList(nullptr)
	{
	}

	inline D3D12Command::~D3D12Command()
	{
	}

}