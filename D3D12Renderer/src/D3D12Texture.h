#pragma once

#include "Hazel/Rendering/Texture.h"

namespace Hazel {

	class HAZEL_API D3D12Texture : public Texture
	{
	public:
		//! CTOR/DTOR:
		D3D12Texture(Renderer* renderer, uint32_t width, uint32_t height, uint32_t depth = 0);
		virtual ~D3D12Texture();

		//! VIRTUALS:
		void Build(const void* data, const CreateInfo& ci) override;
		void Cleanup() override;
	};

	////////////////////////////////////////////////////////////////////////////////
	// D3D12Texture inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline D3D12Texture::D3D12Texture(Renderer* renderer, uint32_t width, uint32_t height, uint32_t depth) :
		Texture(renderer, width, height, depth)
	{
	}

	inline D3D12Texture::~D3D12Texture()
	{
	}

}