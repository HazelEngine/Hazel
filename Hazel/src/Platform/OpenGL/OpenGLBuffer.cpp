#include "hzpch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace Hazel {

	///////////////////////////////////////////////////////////////////////////////////////////////
	// VertexBuffer ///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		HZ_PROFILE_FUNCTION()
		
		glCreateBuffers(1, &m_RendererId);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
		glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
	{
		HZ_PROFILE_FUNCTION()
		
		glCreateBuffers(1, &m_RendererId);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
		glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), vertices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		HZ_PROFILE_FUNCTION()
		glDeleteBuffers(1, &m_RendererId);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		HZ_PROFILE_FUNCTION()
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		HZ_PROFILE_FUNCTION()
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
	{
		HZ_PROFILE_FUNCTION()
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// IndexBuffer ////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		HZ_PROFILE_FUNCTION()
		
		glCreateBuffers(1, &m_RendererId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		HZ_PROFILE_FUNCTION()
		glDeleteBuffers(1, &m_RendererId);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		HZ_PROFILE_FUNCTION()
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		HZ_PROFILE_FUNCTION()
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}