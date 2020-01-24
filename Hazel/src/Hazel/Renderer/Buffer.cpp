#include "hzpch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Hazel {

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
				return nullptr;

			case RendererAPI::API::OpenGL:
				return std::make_shared<OpenGLVertexBuffer>(vertices, count);

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
				return std::make_shared<OpenGLIndexBuffer>(indices, count);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
				return nullptr;
		}
	}

}