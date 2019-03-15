#pragma once

#include "Hazel/Rendering/Buffer.h"

// Forward declarations:
struct ID3D12Resource;

namespace Hazel {

	class HAZEL_API D3D12Buffer : public Buffer
	{
	public:
		friend class D3D12RenderPass;
		friend class D3D12Texture;

		//! CTOR/DTOR:
		D3D12Buffer(Renderer* renderer);
		virtual ~D3D12Buffer();

		//! VIRTUALS:
		void Build(uint32_t size, const void* data, const CreateInfo& ci) override;
		void* Map(uint32_t size, uint32_t offset) override;
		void Unmap() override;
		void Flush() override;
		void Bind(Command* cmd) override;
		void Cleanup() override;

	protected:
		//! MEMBERS:
		ID3D12Resource* m_Buffer;
	};

	////////////////////////////////////////////////////////////////////////////////
	// D3D12Buffer inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline D3D12Buffer::D3D12Buffer(Renderer* renderer) :
		Buffer(renderer),
		m_Buffer(nullptr)
	{
	}

	inline D3D12Buffer::~D3D12Buffer()
	{
		D3D12Buffer::Cleanup();
	}

}