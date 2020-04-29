#include "hzpch.h"
#include "Texture.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/Vulkan/VulkanTexture.h"

#include <stb_image.h>

namespace Hazel {

	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		stbi_set_flip_vertically_on_load(true);

		int width, height, channels;
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		HZ_CORE_ASSERT(data, "Failed to load image!")

		return Create(data, width, height, 4);
	}

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		return Create(nullptr, width, height, 4);
	}

	Ref<Texture2D> Texture2D::Create(
		void* data,
		uint32_t width,
		uint32_t height,
		uint32_t channels
	)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;

			case RendererAPI::API::OpenGL:
				return std::make_shared<OpenGLTexture2D>(data, width, height, channels);

			case RendererAPI::API::Vulkan:
				return std::make_shared<VulkanTexture2D>(data, width, height, channels);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
				return nullptr;
		}
	}

}