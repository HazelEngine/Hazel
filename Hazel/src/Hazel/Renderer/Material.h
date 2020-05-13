#pragma once

#include <Hazel/Core/Buffer.h>

#include <Hazel/Renderer/Shader.h>
#include <Hazel/Renderer/ShaderUniform.h>
#include <Hazel/Renderer/Texture.h>

#include <unordered_set>

namespace Hazel {

	// TODO: Currently the shaders material/instances count are limited,
	// as the uniform buffer has to be allocated with certain amount of memory.
	// In the future, the material data will be baked into files, and then loaded
	// from disk, so we can allocate the required memory as needed.

	enum class MaterialFlag
	{
		None		= BIT(0),
		DepthTest	= BIT(1),
		Blend		= BIT(2)
	};

	class HAZEL_API Material
	{
		friend class MaterialInstance;

	public:
		Material(const Ref<Shader>& shader);
		virtual ~Material();

		void Bind();

		template <typename T>
		void Set(const std::string& name, const T& value)
		{
			auto decl = FindUniformDeclaration(name);
			if (!decl)
			{
				HZ_CORE_ERROR("Could not find param with name '{0}'", name)
				return;
			}
			
			auto& buffer = GetUniformBufferTarget(decl);
			buffer.Write((byte*)&value, decl->GetSize(), decl->GetOffset());

			for (auto instance : m_Instances)
				instance->OnMaterialValueUpdated(decl);
		}

		void Set(const std::string& name, const Ref<Texture>& texture)
		{
			auto resource = FindResourceDeclaration(name);
			m_Textures[resource->GetName()] = texture;
		}

		void Set(const std::string& name, const Ref<Texture2D>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		//void Set(const std::string& name, const Ref<TextureCube>& texture)
		//{
		//	Set(name, (const Ref<Texture>&)texture);
		//}

		uint32_t GetFlags() const { return m_Flags; }
		void SetFlag(MaterialFlag flag) { m_Flags |= (uint32_t)flag; }

		const std::unordered_map<std::string, Ref<Texture>>& GetTextures() const { return m_Textures; }

		uint32_t GetUniformBufferIndex() const { return m_MaterialUniformBufferIndex; }

	public:
		static Ref<Material> Create(const Ref<Shader>& shader);

	private:
		void AllocateStorage();
		void OnShaderReloaded();

		ShaderUniformDeclaration* FindUniformDeclaration(const std::string& name);
		ShaderResourceDeclaration* FindResourceDeclaration(const std::string& name);
		Buffer& GetUniformBufferTarget(ShaderUniformDeclaration* uniform);

	private:
		Ref<Shader> m_Shader;
		std::unordered_set<MaterialInstance*> m_Instances;

		Buffer m_VSUniformStorageBuffer;
		Buffer m_PSUniformStorageBuffer;
		std::unordered_map<std::string, Ref<Texture>> m_Textures;

		// Index of the material data in the shader uniform buffer
		uint32_t m_MaterialUniformBufferIndex;

		uint32_t m_Flags;
	};

	class HAZEL_API MaterialInstance
	{
		friend class Material;

	public:
		MaterialInstance(const Ref<Material>& material);
		virtual ~MaterialInstance();

		void Bind();

		template <typename T>
		void Set(const std::string& name, const T& value)
		{
			auto decl = m_Material->FindUniformDeclaration(name);
			if (!decl)
			{
				HZ_CORE_ERROR("Could not find param with name '{0}'", name)
				return;
			}

			auto& buffer = GetUniformBufferTarget(decl);
			buffer.Write((byte*)&value, decl->GetSize(), decl->GetOffset());

			m_OverriddenValues.insert(name);
		}

		void Set(const std::string& name, const Ref<Texture>& texture)
		{
			auto resource = m_Material->FindResourceDeclaration(name);
			m_Textures[resource->GetName()] = texture;
		}

		void Set(const std::string& name, const Ref<Texture2D>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		//void Set(const std::string& name, const Ref<TextureCube>& texture)
		//{
		//	Set(name, (const Ref<Texture>&)texture);
		//}

		uint32_t GetFlags() const { return m_Material->m_Flags; }
		bool GetFlag(MaterialFlag flag) const { return (uint32_t)flag & m_Material->m_Flags; }
		void SetFlag(MaterialFlag flag, bool value = true);

		Ref<Material> GetMaterial() { return m_Material; }
		Ref<Shader> GetShader() { return m_Material->m_Shader; }
		const std::unordered_map<std::string, Ref<Texture>>& GetTextures() const { return m_Textures; }

		uint32_t GetUniformBufferIndex() const { return m_MaterialUniformBufferIndex; }

	public:
		static Ref<MaterialInstance> Create(const Ref<Material>& material);

	private:
		void AllocateStorage();
		void OnShaderReloaded();
		void OnMaterialValueUpdated(ShaderUniformDeclaration* uniform);

		Buffer& GetUniformBufferTarget(ShaderUniformDeclaration* uniform);

	private:
		Ref<Material> m_Material;

		Buffer m_VSUniformStorageBuffer;
		Buffer m_PSUniformStorageBuffer;
		std::unordered_map<std::string, Ref<Texture>> m_Textures;

		// Index of the instance data in the shader uniform buffer
		uint32_t m_MaterialUniformBufferIndex;

		// TODO: This is temporary; come up with a proper system to track overrides
		std::unordered_set<std::string> m_OverriddenValues;
	};

}