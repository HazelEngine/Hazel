#include "hzpch.h"
#include "Material.h"

namespace Hazel {

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// Material
	/////////////////////////////////////////////////////////////////////////////////////////////////

	Ref<Material> Material::Create(const Ref<Shader>& shader)
	{
		return CreateRef<Material>(shader);
	}

	Material::Material(const Ref<Shader>& shader)
		: m_Shader(shader)
	{
		if (m_Shader->HasVSMaterialUniformBuffer() || m_Shader->HasPSMaterialUniformBuffer())
		{
			uint32_t materialCount = m_Shader->GetMaterialCount();
			m_MaterialUniformBufferIndex = materialCount;
			m_Shader->SetMaterialCount(materialCount + 1);
		}

		AllocateStorage();

		m_Flags |= (uint32_t)MaterialFlag::DepthTest;
		m_Flags |= (uint32_t)MaterialFlag::Blend;
	}

	Material::~Material() {}

	void Material::Bind()
	{
		if (m_VSUniformStorageBuffer)
		{
			m_Shader->SetMaterialUniformBuffer(
				m_VSUniformStorageBuffer,
				m_MaterialUniformBufferIndex
			);
		}
		
		if (m_PSUniformStorageBuffer)
		{
			m_Shader->SetMaterialUniformBuffer(
				m_PSUniformStorageBuffer,
				m_MaterialUniformBufferIndex
			);
		}

		for (auto tex = m_Textures.begin(); tex != m_Textures.end(); tex++)
		{
			if (tex->second)
				m_Shader->BindTextureToPool(tex->first, tex->second);
		}
	}

	void Material::AllocateStorage()
	{
		if (m_Shader->HasVSMaterialUniformBuffer())
		{
			const auto& vsBuffer = m_Shader->GetVSMaterialUniformBuffer();
			m_VSUniformStorageBuffer.Allocate(vsBuffer.GetSize());
			m_VSUniformStorageBuffer.ZeroInitialize();
		}
		
		if (m_Shader->HasPSMaterialUniformBuffer())
		{
			const auto& psBuffer = m_Shader->GetPSMaterialUniformBuffer();
			m_PSUniformStorageBuffer.Allocate(psBuffer.GetSize());
			m_PSUniformStorageBuffer.ZeroInitialize();
		}
	}

	void Material::OnShaderReloaded() {}

	ShaderUniformDeclaration* Material::FindUniformDeclaration(const std::string& name)
	{
		if (m_VSUniformStorageBuffer)
		{
			auto& declarations = m_Shader->GetVSMaterialUniformBuffer().GetUniformDeclarations();
			for (ShaderUniformDeclaration* uniform : declarations)
			{
				if (uniform->GetName() == name)
					return uniform;
			}
		}
		
		if (m_PSUniformStorageBuffer)
		{
			auto& declarations = m_Shader->GetPSMaterialUniformBuffer().GetUniformDeclarations();
			for (ShaderUniformDeclaration* uniform : declarations)
			{
				if (uniform->GetName() == name)
					return uniform;
			}
		}

		return nullptr;
	}

	ShaderResourceDeclaration* Material::FindResourceDeclaration(const std::string& name)
	{
		auto& resources = m_Shader->GetResources();
		for (ShaderResourceDeclaration* resource : resources)
		{
			if (resource->GetName() == name)
				return resource;
		}

		return nullptr;
	}
	
	Buffer& Material::GetUniformBufferTarget(ShaderUniformDeclaration* uniform)
	{
		switch (uniform->GetDomain())
		{
			case ShaderDomain::Vertex:	return m_VSUniformStorageBuffer;
			case ShaderDomain::Pixel:	return m_PSUniformStorageBuffer;
		}
	
		HZ_CORE_ASSERT(false, "Invalid uniform declaration domain! Material does not support this shader type.")
		return m_VSUniformStorageBuffer;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// MaterialInstance
	/////////////////////////////////////////////////////////////////////////////////////////////////

	Ref<MaterialInstance> MaterialInstance::Create(const Ref<Material>& material)
	{
		return CreateRef<MaterialInstance>(material);
	}

	MaterialInstance::MaterialInstance(const Ref<Material>& material)
		: m_Material(material)
	{
		if (m_Material->m_Shader->HasVSMaterialUniformBuffer() ||
			m_Material->m_Shader->HasPSMaterialUniformBuffer())
		{
			uint32_t materialCount = m_Material->m_Shader->GetMaterialCount();
			m_MaterialUniformBufferIndex = materialCount;
			m_Material->m_Shader->SetMaterialCount(materialCount + 1);
		}

		m_Material->m_Instances.insert(this);
		AllocateStorage();
	}

	MaterialInstance::~MaterialInstance()
	{
		m_Material->m_Instances.erase(this);
	}

	void MaterialInstance::Bind()
	{
		if (m_VSUniformStorageBuffer)
		{
			m_Material->m_Shader->SetMaterialUniformBuffer(
				m_VSUniformStorageBuffer,
				m_MaterialUniformBufferIndex
			);
		}
		
		if (m_PSUniformStorageBuffer)
		{
			m_Material->m_Shader->SetMaterialUniformBuffer(
				m_PSUniformStorageBuffer,
				m_MaterialUniformBufferIndex
			);
		}

		for (auto tex = m_Textures.begin(); tex != m_Textures.end(); tex++)
		{
			if (tex->second)
				m_Material->m_Shader->BindTextureToPool(tex->first, tex->second);
		}
	}

	void MaterialInstance::SetFlag(MaterialFlag flag, bool value)
	{
		if (value)
			m_Material->m_Flags |= (uint32_t)flag;
		else
			m_Material->m_Flags &= ~(uint32_t)flag;
	}

	void MaterialInstance::AllocateStorage()
	{
		if (m_Material->m_Shader->HasVSMaterialUniformBuffer())
		{
			const auto& vsBuffer = m_Material->m_Shader->GetVSMaterialUniformBuffer();
			m_VSUniformStorageBuffer = Buffer::Copy(
				m_Material->m_VSUniformStorageBuffer.Data,
				vsBuffer.GetSize()
			);
		}
		
		if (m_Material->m_Shader->HasPSMaterialUniformBuffer())
		{
			const auto& psBuffer = m_Material->m_Shader->GetPSMaterialUniformBuffer();
			m_PSUniformStorageBuffer = Buffer::Copy(
				m_Material->m_PSUniformStorageBuffer.Data,
				psBuffer.GetSize()
			);
		}
	}

	void MaterialInstance::OnShaderReloaded() {}

	void MaterialInstance::OnMaterialValueUpdated(ShaderUniformDeclaration* uniform)
	{
		if (m_OverriddenValues.find(uniform->GetName()) == m_OverriddenValues.end())
		{
			auto& buffer = GetUniformBufferTarget(uniform);
			auto& matBuffer = m_Material->GetUniformBufferTarget(uniform);
			buffer.Write(matBuffer.Data + uniform->GetOffset(), uniform->GetSize(), uniform->GetOffset());
		}
	}

	Buffer& MaterialInstance::GetUniformBufferTarget(ShaderUniformDeclaration* uniform)
	{
		switch (uniform->GetDomain())
		{
			case ShaderDomain::Vertex:	return m_VSUniformStorageBuffer;
			case ShaderDomain::Pixel:	return m_PSUniformStorageBuffer;
		}
	
		HZ_CORE_ASSERT(false, "Invalid uniform declaration domain! Material does not support this shader type.")
		return m_VSUniformStorageBuffer;
	}

}