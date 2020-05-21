#include "hzpch.h"
#include "VulkanShaderUniform.h"

namespace Hazel {

	VulkanShaderResourceDeclaration::VulkanShaderResourceDeclaration(
		const std::string& name,
		uint32_t set,
		uint32_t binding,
		ShaderDomain domain,
		Type type,
		Dimension dimension,
		uint32_t count
	) : m_Name(name),
		m_Set(set),
		m_Binding(binding),
		m_Domain(domain),
		m_Type(type),
		m_Dimension(dimension),
		m_Count(count) {}

	VulkanShaderUniformDeclaration::VulkanShaderUniformDeclaration(const std::string& name, ShaderDomain domain, Type type, uint32_t count)
		: m_Name(name), m_Domain(domain), m_Type(type), m_Count(count)
	{
		m_Size = SizeOfUniformType(type) * count;
	}

	uint32_t VulkanShaderUniformDeclaration::SizeOfUniformType(Type type)
	{
		switch (type)
		{
			case VulkanShaderUniformDeclaration::Type::Int32:      return 4;
			case VulkanShaderUniformDeclaration::Type::Float32:    return 4;
			case VulkanShaderUniformDeclaration::Type::Vec2:       return 4 * 2;
			case VulkanShaderUniformDeclaration::Type::Vec3:       return 4 * 3;
			case VulkanShaderUniformDeclaration::Type::Vec4:       return 4 * 4;
			case VulkanShaderUniformDeclaration::Type::Mat3:       return 4 * 3 * 3;
			case VulkanShaderUniformDeclaration::Type::Mat4:       return 4 * 4 * 4;
		}

		return 0;
	}

	void VulkanShaderUniformDeclaration::SetOffset(uint32_t offset)
	{
		m_Offset = offset;
	}

	VulkanShaderUniformBufferDeclaration::VulkanShaderUniformBufferDeclaration(const std::string& name, uint32_t binding, ShaderDomain domain)
		: m_Name(name), m_Binding(binding), m_Domain(domain), m_Size(0) {}

	void VulkanShaderUniformBufferDeclaration::PushUniform(VulkanShaderUniformDeclaration* uniform)
	{
		uint32_t offset = 0;
		if (m_Uniforms.size())
		{
			VulkanShaderUniformDeclaration* previous = (VulkanShaderUniformDeclaration*)m_Uniforms.back();
			offset = previous->m_Offset + previous->m_Size;
		}
		uniform->SetOffset(offset);
		m_Size += uniform->GetSize();
		m_Uniforms.push_back(uniform);
	}

	ShaderUniformDeclaration* VulkanShaderUniformBufferDeclaration::FindUniform(const std::string& name)
	{
		for (ShaderUniformDeclaration* uniform : m_Uniforms)
		{
			if (uniform->GetName() == name)
				return uniform;
		}

		return nullptr;
	}

}