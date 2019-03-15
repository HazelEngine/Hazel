#pragma once

#include "Hazel/Core.h"

#include "Hazel/Rendering/Buffer.h"
#include "Hazel/Rendering/Color.h"
#include "Hazel/Rendering/Format.h"

namespace Hazel {

	// Forward declarations:
	class Renderer;

	class HAZEL_API Texture
	{
	public:
		//! TYPEDEF/ENUMS:
		enum Type
		{
			eTexture1D,
			eTexture2D,
			eTexture3D,
			eTextureCube,
			eTexture1DArray,
			eTexture2DArray,
			eTextureCubeArray
		};

		enum UsageFlags
		{
			eTransferSrc            = 1 << 1,
			eTransferDst            = 1 << 2,
			eSampled                = 1 << 3,
			eStorage                = 1 << 4,
			eColorAttachment        = 1 << 5,
			eDepthStencilAttachment = 1 << 6,
			eTransientAttachment    = 1 << 7,
			eInputAttachment        = 1 << 8
		};

		struct CreateInfo
		{
			//! SERVICES:
			static CreateInfo GetDefault();

			//! MEMBERS:
			uint32_t Usage;
			uint32_t MemProps;
			Type Type_;
			Format Format_;
			uint32_t LayerCount;
			Color ClearColor;
		};

		//! CTOR/DTOR:
		Texture(Renderer* renderer, uint32_t width, uint32_t height, uint32_t depth);
		virtual ~Texture();

		//! VIRTUALS:
		virtual void Build(const void* data, const CreateInfo& ci) = 0;
		virtual void Cleanup() = 0;

		//! ACCESSORS:
		Renderer* GetRenderer() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		uint32_t GetDepth() const;
		Format GetFormat() const;
		uint32_t GetLayerCount() const;
		const Color& GetClearColor() const;

	protected:
		//! MEMBERS:
		Renderer* m_Renderer;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_Depth;
		Format m_Format;
		uint32_t m_LayerCount;
		Color m_ClearColor;
	};

	////////////////////////////////////////////////////////////////////////////////
	// Texture::CreateInfo inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline Texture::CreateInfo Texture::CreateInfo::GetDefault()
	{
		return CreateInfo() =
		{
			Texture::eSampled | Texture::eTransferDst,
			Buffer::eDeviceLocal,
			Texture::eTexture2D,
			Format::eRGBA8,
			1,
			Color::Black
		};
	}

	////////////////////////////////////////////////////////////////////////////////
	// Texture inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline Texture::Texture(Renderer* renderer, uint32_t width, uint32_t height, uint32_t depth)
	{
	}

	inline Texture::~Texture()
	{
	}

	inline Renderer* Texture::GetRenderer() const
	{
		return m_Renderer;
	}

	inline uint32_t Texture::GetWidth() const
	{
		return m_Width;
	}

	inline uint32_t Texture::GetHeight() const
	{
		return m_Height;
	}

	inline uint32_t Texture::GetDepth() const
	{
		return m_Depth;
	}

	inline Format Texture::GetFormat() const
	{
		return m_Format;
	}

	inline uint32_t Texture::GetLayerCount() const
	{
		return m_LayerCount;
	}

	inline const Color& Texture::GetClearColor() const
	{
		return m_ClearColor;
	}

}