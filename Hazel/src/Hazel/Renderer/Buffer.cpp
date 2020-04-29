#include "hzpch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Hazel {

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
				return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLVertexBuffer>(size);

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanVertexBuffer>(size);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
				return nullptr;
		}
	}

	Ref<VertexBuffer> VertexBuffer::Create(void* data, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
				return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLVertexBuffer>(data, size);

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanVertexBuffer>(data, size);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
				return nullptr;
		}
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
				return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLIndexBuffer>(indices, count);

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanIndexBuffer>(indices, count);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
				return nullptr;
		}
	}

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
				return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLUniformBuffer>(size);

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanUniformBuffer>(size);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
				return nullptr;
		}
	}

	Ref<UniformBuffer> UniformBuffer::Create(void* data, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
				return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLUniformBuffer>(data, size);

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanUniformBuffer>(data, size);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
				return nullptr;
		}
	}

}