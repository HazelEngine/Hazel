#include "hzpch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Hazel {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case Hazel::ShaderDataType::Bool:      return GL_BOOL;
			case Hazel::ShaderDataType::Float:     return GL_FLOAT;
			case Hazel::ShaderDataType::Float2:    return GL_FLOAT;
			case Hazel::ShaderDataType::Float3:    return GL_FLOAT;
			case Hazel::ShaderDataType::Float4:    return GL_FLOAT;
			case Hazel::ShaderDataType::Int:       return GL_INT;
			case Hazel::ShaderDataType::Int2:      return GL_INT;
			case Hazel::ShaderDataType::Int3:      return GL_INT;
			case Hazel::ShaderDataType::Int4:      return GL_INT;
			case Hazel::ShaderDataType::Mat3:      return GL_FLOAT;
			case Hazel::ShaderDataType::Mat4:      return GL_FLOAT;
		}

		HZ_CORE_ASSERT(false, "Unknown ShaderDataType!")
			return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererId);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererId);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererId);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		glBindVertexArray(m_RendererId);
		vertexBuffer->Bind();

		HZ_CORE_ASSERT(
			vertexBuffer->GetLayout().GetElements().size(),
			"Vertex buffer has no layout!"
		)

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(
				index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(GLvoid*)element.Offset
			);
			index++;
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererId);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

}