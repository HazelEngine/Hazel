#pragma once

#include "Hazel/Rendering/RenderPass.h"

// Forward declarations:
struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct ID3D12DescriptorHeap;

namespace Hazel {

	class HAZEL_API D3D12RenderPass : public RenderPass
	{
	public:
		friend class D3D12Buffer;
		friend class D3D12Command;
		friend class D3D12Renderer;

		//! CTOR/DTOR:
		D3D12RenderPass(Renderer* renderer, RenderPass* parent);
		virtual ~D3D12RenderPass();

		//! VIRTUALS:
		void Build(const CreateInfo& ci) override;
		void Resize(int width, int height) override;
		void Record() override;
		void Cleanup() override;

	protected:
		//! SERVICES:
		void CreateCommandList();
		void CreateRootSignature();
		void CreatePipelineStateObject();
		D3D_PRIMITIVE_TOPOLOGY ConvertToDxTopology(Mesh::Topology topology);

		//! MEMBERS:
		ID3D12GraphicsCommandList* m_CommandList;
		ID3D12RootSignature* m_RootSignature;
		ID3D12PipelineState* m_PipelineStateObject;
		ID3D12DescriptorHeap* m_CbvSrvHeap;
		ID3D12DescriptorHeap* m_RtvHeap;
		std::vector<std::shared_ptr<Command>> m_Commands;
		ivec2 m_Resolution;
	};

	////////////////////////////////////////////////////////////////////////////////
	// D3D12RenderPass inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline D3D12RenderPass::D3D12RenderPass(Renderer* renderer, RenderPass* parent) :
		RenderPass(renderer, parent),
		m_CommandList(nullptr),
		m_RootSignature(nullptr),
		m_PipelineStateObject(nullptr),
		m_CbvSrvHeap(nullptr),
		m_RtvHeap(nullptr)
	{
	}

	inline D3D12RenderPass::~D3D12RenderPass()
	{
	}

}