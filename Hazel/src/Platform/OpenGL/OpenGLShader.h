#pragma once

#include <Hazel/Renderer/Shader.h>
#include <Hazel/Renderer/Buffer.h>

// TODO: REMOVE!
typedef unsigned int GLenum;

namespace Hazel {

	class OpenGLShader : public Shader
	{
	public:
		struct ShaderResource
		{
			uint32_t Binding;
			std::string Name;
			uint32_t Size;

			bool IsBuffer = false;
			bool IsImage = false;
		};

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

		virtual void SetUniformBuffer(const std::string& name, void* data, uint32_t size) override;

		virtual void BindTexture(const std::string& name, const Ref<Texture2D>& texture) override;
		virtual Ref<Texture2D> GetTexture(const std::string& name) const override;

		const std::string& GetName() const override { return m_Name; };

		uint32_t GetRendererId() const { return m_RendererId; }

		std::vector<Ref<Texture2D>> GetTextures() const;
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

		void GetShaderResources(const std::vector<uint32_t>& spirv);
		void GenerateShaderResources();

	private:
		uint32_t m_RendererId;
		std::string m_Name;

		std::vector<ShaderResource> m_ShaderResources;
		std::unordered_map<std::string, Ref<UniformBuffer>> m_UniformBuffers;
		std::unordered_map<std::string, Ref<Texture2D>> m_Textures;
	};

}