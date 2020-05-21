#pragma once

#include "Hazel/Renderer/Buffer.h"

#include <glad/glad.h>

namespace Hazel {

	class HAZEL_API OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(void* data, uint32_t size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void* Map() override { return m_LocalBuffer; }
		virtual void Unmap(uint32_t size) override;

		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

		virtual uint32_t GetRendererId() const { return m_RendererId; }

	private:
		uint32_t m_RendererId;
		BufferLayout m_Layout;

		uint32_t m_Size;
		char* m_LocalBuffer = nullptr;
	};

	class HAZEL_API OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(void* data, uint32_t size);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual uint32_t GetSize() const override { return m_Size; }

		virtual uint32_t GetRendererId() const { return m_RendererId; }

	private:
		uint32_t m_RendererId;
		uint32_t m_Size;
	};

	class HAZEL_API OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(uint32_t size);
		OpenGLUniformBuffer(void* data, uint32_t size);
		virtual ~OpenGLUniformBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void* Map() override { return m_LocalBuffer; }
		virtual void Unmap(uint32_t size) override;
		virtual void Unmap(uint32_t offset, uint32_t size) override;

		uint32_t GetRendererId() const { return m_RendererId; }
		
		uint32_t GetBinding() const { return m_Binding; }
		void SetBinding(uint32_t binding) { m_Binding = binding; }

		uint32_t GetSize() const { return m_Size; }

	private:
		uint32_t m_RendererId;
		uint32_t m_Binding;

		uint32_t m_Size;
		char* m_LocalBuffer = nullptr;
	};

	inline GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case Hazel::ShaderDataType::Float:		return GL_FLOAT;
		case Hazel::ShaderDataType::Float2:		return GL_FLOAT;
		case Hazel::ShaderDataType::Float3:		return GL_FLOAT;
		case Hazel::ShaderDataType::Float4:		return GL_FLOAT;
		case Hazel::ShaderDataType::Int:		return GL_INT;
		case Hazel::ShaderDataType::Int2:		return GL_INT;
		case Hazel::ShaderDataType::Int3:		return GL_INT;
		case Hazel::ShaderDataType::Int4:		return GL_INT;
		}
	}

}