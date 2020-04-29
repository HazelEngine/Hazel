#include "hzpch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace Hazel {

	///////////////////////////////////////////////////////////////////////////////////////////////
	// VertexBuffer ///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
		: m_Size(size)
	{
		HZ_PROFILE_FUNCTION()

		m_LocalBuffer = new char[size];
		glCreateBuffers(1, &m_RendererId);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(void* data, uint32_t size)
		: m_Size(size)
	{
		HZ_PROFILE_FUNCTION()
		
		glCreateBuffers(1, &m_RendererId);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		HZ_PROFILE_FUNCTION()
		glDeleteBuffers(1, &m_RendererId);
		delete[] m_LocalBuffer;
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

	void OpenGLVertexBuffer::Unmap(uint32_t size)
	{
		HZ_PROFILE_FUNCTION()
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, m_LocalBuffer);
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

	///////////////////////////////////////////////////////////////////////////////////////////////
	// UniformBuffer //////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size)
		: m_Size(size), m_Binding(0)
	{
		HZ_PROFILE_FUNCTION()
		
		m_LocalBuffer = new char[size];
		glCreateBuffers(1, &m_RendererId);
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererId);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLUniformBuffer::OpenGLUniformBuffer(void* data, uint32_t size)
		: m_Size(size), m_Binding(0)
	{
		HZ_PROFILE_FUNCTION()
		
		m_LocalBuffer = new char[size];
		glCreateBuffers(1, &m_RendererId);
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererId);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		HZ_PROFILE_FUNCTION()
		glDeleteBuffers(1, &m_RendererId);
		delete[] m_LocalBuffer;
	}

	void OpenGLUniformBuffer::Bind() const
	{
		HZ_PROFILE_FUNCTION()
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererId);
	}

	void OpenGLUniformBuffer::Unbind() const
	{
		HZ_PROFILE_FUNCTION()
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenGLUniformBuffer::Unmap(uint32_t size)
	{
		HZ_PROFILE_FUNCTION()
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererId);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, m_LocalBuffer);
	}

}