#pragma once

#include <Hazel/Renderer/ShaderUniform.h>

namespace Hazel {

	class OpenGLShaderResourceDeclaration : public ShaderResourceDeclaration
	{
		friend class OpenGLShader;

		enum class Type
		{
			None, SampledImage, Image, Sampler
		};

		enum class Dimension
		{
			None, Texture2D, TextureCube
		};

	public:
		OpenGLShaderResourceDeclaration(const std::string& name, uint32_t binding, ShaderDomain domain, Type type, Dimension dimension, uint32_t count);

		inline const std::string& GetName() const override { return m_Name; }
		inline uint32_t GetBinding() const override { return m_Binding; }
		inline uint32_t GetCount() const override { return m_Count; }
		inline ShaderDomain GetDomain() const override { return m_Domain; }

		inline Type GetType() const { return m_Type; }
		inline Dimension GetDimension() const { return m_Dimension; }

	private:
		std::string m_Name;
		uint32_t m_Binding;
		uint32_t m_Count;

		Type m_Type;
		Dimension m_Dimension;
		ShaderDomain m_Domain;
	};

	class OpenGLShaderUniformDeclaration : public ShaderUniformDeclaration
	{
		friend class OpenGLShader;
		friend class OpenGLShaderUniformBufferDeclaration;

		enum class Type
		{
			None, Float32, Vec2, Vec3, Vec4, Mat3, Mat4, Int32
		};

	public:
		OpenGLShaderUniformDeclaration(const std::string& name, ShaderDomain domain, Type type, uint32_t count);

		inline const std::string& GetName() const override { return m_Name; }
		inline uint32_t GetSize() const override { return m_Size; }
		inline uint32_t GetCount() const override { return m_Count; }
		inline uint32_t GetOffset() const override { return m_Offset; }
		inline ShaderDomain GetDomain() const override { return m_Domain; }

		inline Type GetType() const { return m_Type; }
		inline bool IsArray() const { return m_Count > 1; }

	public:
		static uint32_t SizeOfUniformType(Type type);

	protected:
		void SetOffset(uint32_t offset) override;

	private:
		std::string m_Name;
		uint32_t m_Size;
		uint32_t m_Count;
		uint32_t m_Offset;
		ShaderDomain m_Domain;
		Type m_Type;
	};

	class OpenGLShaderUniformBufferDeclaration : public ShaderUniformBufferDeclaration
	{
		friend class Shader;

	public:
		OpenGLShaderUniformBufferDeclaration(const std::string& name, uint32_t binding, ShaderDomain domain);

		void PushUniform(OpenGLShaderUniformDeclaration* uniform);

		inline const std::string& GetName() const override { return m_Name; }
		inline uint32_t GetBinding() const override { return m_Binding; }
		inline uint32_t GetSize() const override { return m_Size; }
		inline const ShaderUniformList& GetUniformDeclarations() const override { return m_Uniforms; }
		inline ShaderDomain GetDomain() const override { return m_Domain; }

		ShaderUniformDeclaration* FindUniform(const std::string& name) override;

	private:
		std::string m_Name;
		ShaderUniformList m_Uniforms;
		uint32_t m_Binding;
		uint32_t m_Size;
		ShaderDomain m_Domain;
	};

}