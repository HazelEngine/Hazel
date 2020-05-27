#pragma once

#include <Hazel/Core/Buffer.h>
#include <Hazel/Renderer/Texture.h>
#include <Hazel/Renderer/ShaderUniform.h>

#include <string>
#include <glm/glm.hpp>

namespace Hazel {

	struct ShaderCreateInfo
	{
		std::string Name;
		std::vector<uint32_t> VertexShaderSource;
		std::vector<uint32_t> FragmentShaderSource;
	};

	class HAZEL_API Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;

		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;

		virtual void SetMat3(const std::string& name, const glm::mat3& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual const ShaderUniformBufferList GetVSUniformBuffers() const = 0;
		virtual const ShaderUniformBufferList GetPSUniformBuffers() const = 0;
		virtual const ShaderResourceList GetResources() const = 0;
		
		virtual void SetUniformBuffer(const std::string& name, void* data, uint32_t size) = 0;
		virtual void SetUniformBufferParam(const std::string& name, const std::string& param, void* data, uint32_t size) = 0;

		virtual void BindTexture(const std::string& name, const Ref<Texture>& texture) = 0;
		virtual void BindTexture(const std::string& name, uint32_t index, const Ref<Texture>& texture) = 0;
		
		virtual void BindTextureToPool(const std::string& name, const Ref<Texture>& texture) = 0;
		
		virtual Ref<Texture> GetTexture(const std::string& name) const = 0;
		virtual Ref<Texture> GetTexture(const std::string& name, uint32_t index) const = 0;

		// Specific to Material uniforms
		virtual bool HasVSMaterialUniformBuffer() const = 0;
		virtual bool HasPSMaterialUniformBuffer() const = 0;
		virtual const ShaderUniformBufferDeclaration& GetVSMaterialUniformBuffer() const = 0;
		virtual const ShaderUniformBufferDeclaration& GetPSMaterialUniformBuffer() const = 0;
		virtual void SetMaterialUniformBuffer(Buffer buffer, uint32_t materialIndex) = 0;
		
		virtual uint32_t GetMaterialCount() const = 0;
		virtual void SetMaterialCount(uint32_t count) = 0;

		virtual const std::string& GetName() const = 0;

		static Ref<Shader> Create(const ShaderCreateInfo& info);

		static Ref<Shader> Create(
			const std::string& name, 
			const std::string& vsPath,
			const std::string& fsPath
		);
	};

	// This should be eventually be handled by the Asset Manager
	class ShaderLibrary
	{
	public:
		ShaderLibrary();
		~ShaderLibrary();

		void Add(const Ref<Shader>& shader);
		void Load(const std::string& vsPath, const std::string& fsPath);
		void Load(const std::string& name, const std::string& vsPath, const std::string& fsPath);
		void Load(ShaderCreateInfo& info);

		Ref<Shader>& Get(const std::string& name);

	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};

}