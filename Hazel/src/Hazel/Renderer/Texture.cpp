#include "hzpch.h"
#include "Texture.h"

#include <Hazel/Core/Color.h>

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace Hazel {

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Texture
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	uint32_t Texture::GetBPP(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::RGB:		return 3;
			case TextureFormat::RGBA:		return 4;
			default:						return 0;
		}
	}

	uint32_t Texture::CalculateMipMapCount(uint32_t width, uint32_t height)
	{
		uint32_t levels = 1;
		while ((width | height) >> levels)
			levels++;

		return levels;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Texture2D
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	Ref<Texture2D> Texture2D::Create(
		TextureFormat format,
		uint32_t width,
		uint32_t height,
		TextureWrap wrap)
	{
		HZ_CORE_ASSERT(!(format == TextureFormat::None), "Texture must have a format!")
		uint32_t color = format == TextureFormat::RGB ? Color::Magenta : Color::MagentaAlpha;
		return Create(format, width, height, (void*)&color, wrap);
	}

	Ref<Texture2D> Texture2D::Create(
		TextureFormat format,
		uint32_t width,
		uint32_t height,
		void* data,
		TextureWrap wrap
	)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLTexture2D>(format, width, height, data, wrap);

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanTexture2D>(format, width, height, data, wrap);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
				return nullptr;
		}
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path, bool srgb)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLTexture2D>(path, srgb);

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanTexture2D>(path, srgb);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
				return nullptr;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// TextureCube
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	Ref<TextureCube> TextureCube::Create(TextureFormat format, uint32_t width, uint32_t height)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLTextureCube>(format, width, height);

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanTextureCube>(format, width, height);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
				return nullptr;
		}
	}

	Ref<TextureCube> TextureCube::Create(const std::string& path)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;

			case RendererAPI::API::OpenGL: return CreateRef<OpenGLTextureCube>(path);

			case RendererAPI::API::Vulkan: return CreateRef<VulkanTextureCube>(path);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
				return nullptr;
		}
	}

}