#pragma once

#include <Hazel/Renderer/Shader.h>
#include <Hazel/Renderer/Buffer.h>

#include <Platform/OpenGL/OpenGLShaderUniform.h>

#define MAX_MAT_INSTANCES 20

typedef unsigned int GLenum;

namespace Hazel {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const ShaderCreateInfo& info);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;

		virtual void SetMat3(const std::string& name, const glm::mat3& value) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

		virtual const ShaderUniformBufferList GetVSUniformBuffers() const override { return m_VSUniformBuffers; }
		virtual const ShaderUniformBufferList GetPSUniformBuffers() const override { return m_PSUniformBuffers; }
		virtual const ShaderResourceList GetResources() const override { return m_Resources; }

		virtual void SetUniformBuffer(const std::string& name, void* data, uint32_t size) override;

		virtual void BindTexture(const std::string& name, const Ref<Texture>& texture) override;
		virtual void BindTexture(const std::string& name, uint32_t index, const Ref<Texture>& texture) override;
		
		virtual void BindTextureToPool(const std::string& name, const Ref<Texture>& texture) override;
		
		virtual Ref<Texture> GetTexture(const std::string& name) const override;
		virtual Ref<Texture> GetTexture(const std::string& name, uint32_t index) const override;

		// Specific to Material uniforms
		virtual bool HasVSMaterialUniformBuffer() const override { return (bool)m_VSMaterialUniformBuffer; }
		virtual bool HasPSMaterialUniformBuffer() const override { return (bool)m_PSMaterialUniformBuffer; }
		virtual const ShaderUniformBufferDeclaration& GetVSMaterialUniformBuffer() const override { return *m_VSMaterialUniformBuffer; }
		virtual const ShaderUniformBufferDeclaration& GetPSMaterialUniformBuffer() const override { return *m_PSMaterialUniformBuffer; }
		virtual void SetMaterialUniformBuffer(Buffer buffer, uint32_t materialIndex) override;
		
		UniformBuffer* GetMaterialUniformBuffer() const { return m_MaterialUniformBuffer.get(); }
		uint32_t GetMaterialUniformBufferAlignment() const { return m_MaterialUniformBufferAlignment; }

		virtual uint32_t GetMaterialCount() const override { return m_MaterialCount; }
		virtual void SetMaterialCount(uint32_t count) override { m_MaterialCount = count; }

		const std::string& GetName() const override { return m_Name; };

		uint32_t GetRendererId() const { return m_RendererId; }

		// TODO: Temporary, it will be removed when GetTextures() returns all textures in the arrays
		std::vector<Ref<Texture>> GetTexturesVector() const;

		// TODO: Temporary, this should return the textures map as it are
		std::unordered_map<std::string, Ref<Texture>> GetTextures() const;
		std::vector<Ref<UniformBuffer>> GetUniformBuffers() const;

		void UploadUniformInt(const std::string& name, int value) const;
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count) const;

		void UploadUniformFloat(const std::string& name, float value) const;
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value) const;
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value) const;
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value) const;

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const;
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const;

	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSrcs);

		void GetShaderResources(const std::vector<uint32_t>& spirv, ShaderDomain domain);
		void GenerateShaderResources();

	private:
		uint32_t m_RendererId;
		std::string m_Name;

		// Shader resource declarations
		ShaderUniformBufferList m_VSUniformBuffers;
		ShaderUniformBufferList m_PSUniformBuffers;
		ShaderResourceList m_Resources;

		// Specific material resource declarations
		Scope<OpenGLShaderUniformBufferDeclaration> m_VSMaterialUniformBuffer;
		Scope<OpenGLShaderUniformBufferDeclaration> m_PSMaterialUniformBuffer;
		Scope<UniformBuffer> m_MaterialUniformBuffer;
		uint32_t m_MaterialUniformBufferAlignment;
		uint32_t m_MaterialCount;

		// Shader resources
		std::unordered_map<std::string, Ref<UniformBuffer>> m_UniformBuffers;
		std::unordered_map<std::string, std::vector<Ref<Texture>>> m_Textures;
	};

}