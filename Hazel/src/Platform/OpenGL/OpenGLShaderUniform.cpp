#include "hzpch.h"
#include "OpenGLShaderUniform.h"

namespace Hazel {

	OpenGLShaderResourceDeclaration::OpenGLShaderResourceDeclaration(const std::string& name, uint32_t binding, ShaderDomain domain, Type type, Dimension dimension, uint32_t count)
		: m_Name(name), m_Binding(binding), m_Domain(domain), m_Type(type), m_Dimension(dimension), m_Count(count) {}

	OpenGLShaderUniformDeclaration::OpenGLShaderUniformDeclaration(const std::string& name, ShaderDomain domain, Type type, uint32_t count)
		: m_Name(name), m_Domain(domain), m_Type(type), m_Count(count)
	{
		m_Size = SizeOfUniformType(type) * count;
	}

	uint32_t OpenGLShaderUniformDeclaration::SizeOfUniformType(Type type)
	{
		switch (type)
		{
			case OpenGLShaderUniformDeclaration::Type::Int32:      return 4;
			case OpenGLShaderUniformDeclaration::Type::Float32:    return 4;
			case OpenGLShaderUniformDeclaration::Type::Vec2:       return 4 * 2;
			case OpenGLShaderUniformDeclaration::Type::Vec3:       return 4 * 3;
			case OpenGLShaderUniformDeclaration::Type::Vec4:       return 4 * 4;
			case OpenGLShaderUniformDeclaration::Type::Mat3:       return 4 * 3 * 3;
			case OpenGLShaderUniformDeclaration::Type::Mat4:       return 4 * 4 * 4;
		}

		return 0;
	}

	void OpenGLShaderUniformDeclaration::SetOffset(uint32_t offset)
	{
		m_Offset = offset;
	}

	OpenGLShaderUniformBufferDeclaration::OpenGLShaderUniformBufferDeclaration(const std::string& name, uint32_t binding, ShaderDomain domain)
		: m_Name(name), m_Binding(binding), m_Domain(domain), m_Size(0) {}

	void OpenGLShaderUniformBufferDeclaration::PushUniform(OpenGLShaderUniformDeclaration* uniform)
	{
		uint32_t offset = 0;
		if (m_Uniforms.size())
		{
			OpenGLShaderUniformDeclaration* previous = (OpenGLShaderUniformDeclaration*)m_Uniforms.back();
			offset = previous->m_Offset + previous->m_Size;
		}
		uniform->SetOffset(offset);
		m_Size += uniform->GetSize();
		m_Uniforms.push_back(uniform);
	}

	ShaderUniformDeclaration* OpenGLShaderUniformBufferDeclaration::FindUniform(const std::string& name)
	{
		for (ShaderUniformDeclaration* uniform : m_Uniforms)
		{
			if (uniform->GetName() == name)
				return uniform;
		}

		return nullptr;
	}

}