#include "hzpch.h"
#include "OpenGLShader.h"

#include "OpenGLBuffer.h"
#include "OpenGLTexture.h"

#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <spirv_glsl.hpp>

namespace Hazel {

	namespace {
	
		std::string ConvertSpirvToGLSL(const std::vector<uint32_t>& spirv)
		{
			spirv_cross::CompilerGLSL::Options options;
			options.version = 330;
			options.es = false;
			//options.emit_uniform_buffer_as_plain_uniforms = true;

			spirv_cross::CompilerGLSL compiler(spirv);
			compiler.set_common_options(options);

			// Map all combinations of images and samplers, as OpenGL has no separate samplers
			compiler.build_combined_image_samplers();

			// Set the name of the combined sampler to the image name
			for (auto& remap : compiler.get_combined_image_samplers())
			{
				compiler.set_name(remap.combined_id, compiler.get_name(remap.image_id));
			}

			return compiler.compile();
		}

		static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;

		HZ_CORE_ASSERT(false, "Unknown shader type!")
		return 0;
	}

	}

	OpenGLShader::OpenGLShader(const ShaderCreateInfo& info)
		: m_Name(info.Name), m_MaterialCount(0)
	{
		HZ_PROFILE_FUNCTION()
		
		std::unordered_map<GLenum, std::string> shaderSrcs =
		{
			{ GL_VERTEX_SHADER,   ConvertSpirvToGLSL(info.VertexShaderSource)   },
			{ GL_FRAGMENT_SHADER, ConvertSpirvToGLSL(info.FragmentShaderSource) }
		};

		Compile(shaderSrcs);

		GetShaderResources(info.VertexShaderSource, ShaderDomain::Vertex);
		GetShaderResources(info.FragmentShaderSource, ShaderDomain::Pixel);

		GenerateShaderResources();
	}

	OpenGLShader::~OpenGLShader()
	{
		HZ_PROFILE_FUNCTION()
		glDeleteProgram(m_RendererId);
	}

	void OpenGLShader::Bind() const
	{
		HZ_PROFILE_FUNCTION()
		glUseProgram(m_RendererId);
	}

	void OpenGLShader::Unbind() const
	{
		HZ_PROFILE_FUNCTION()
		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		HZ_PROFILE_FUNCTION()
		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
		HZ_PROFILE_FUNCTION()
		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		HZ_PROFILE_FUNCTION()
		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		HZ_PROFILE_FUNCTION()
		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		HZ_PROFILE_FUNCTION()
		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		HZ_PROFILE_FUNCTION()
		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& value)
	{
		HZ_PROFILE_FUNCTION()
		UploadUniformMat3(name, value);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		HZ_PROFILE_FUNCTION()
		UploadUniformMat4(name, value);
	}

	void OpenGLShader::SetUniformBuffer(const std::string& name, void* data, uint32_t size)
	{
		if (m_UniformBuffers.find(name) != m_UniformBuffers.end())
		{
			void* mapped = m_UniformBuffers[name]->Map();
			memcpy(mapped, data, size);
			m_UniformBuffers[name]->Unmap(size);
			return;
		}

		HZ_CORE_ERROR("Shader uniform buffer {0} doesn't exists!", name)
	}

	void OpenGLShader::SetUniformBufferParam(const std::string& name, const std::string& param, void* data, uint32_t size)
	{
		ShaderUniformBufferDeclaration* ubuffer = nullptr;

		for (auto decl : m_VSUniformBuffers)
		{
			if (decl->GetName() == name)
				ubuffer = decl;
		}
		for (auto decl : m_PSUniformBuffers)
		{
			if (decl->GetName() == name)
				ubuffer = decl;
		}

		if (ubuffer)
		{
			auto paramDecl = ubuffer->FindUniform(param);
			if (paramDecl)
			{
				if (size == paramDecl->GetSize())
				{
					uint32_t offset = paramDecl->GetOffset();
					byte* mapped = (byte*)m_UniformBuffers[name]->Map();
					memcpy(mapped + offset, data, size);
					m_UniformBuffers[name]->Unmap(offset, size);
					return;
				}
				HZ_CORE_ERROR("Size for param '{0}' has to be equal to {1}, currently is {2}.", param, paramDecl->GetSize(), size)
				return;
			}
			HZ_CORE_ERROR("Param '{0}' doesn't exists for uniform buffer '{1}'!", param, name)
			return;
		}
		HZ_CORE_ERROR("Shader uniform buffer {0} doesn't exists!", name)
	}

	void OpenGLShader::BindTexture(const std::string& name, const Ref<Texture>& texture)
	{
		BindTexture(name, 0, texture);
	}

	void OpenGLShader::BindTexture(const std::string& name, uint32_t index, const Ref<Texture>& texture)
	{
		for (auto resource : m_Resources)
		{
			if (resource->GetName() == name)
			{
				if (index > resource->GetCount())
				{
					HZ_CORE_ERROR("Has no slot {0} in the texture {1}.", index, name)
					return;
				}

				// Get the textures, update the texture in the index, and set textures back
				auto& textures = m_Textures[name];
				textures[index] = texture;
				m_Textures[name] = textures;

				// Set the Sampler name
				auto gl_Texture = dynamic_cast<OpenGLBaseTexture*>(texture.get());
				gl_Texture->SetSamplerName(name + "[" + std::to_string(index) + "]");

				return;
			}
		}

		HZ_CORE_ERROR("Has no {0} texture in the shader {1}", name, m_Name)
	}

	void OpenGLShader::BindTextureToPool(const std::string& name, const Ref<Texture>& texture)
	{
		for (auto resource : m_Resources)
		{
			if (resource->GetName() == name)
			{
				// Set the Sampler name
				auto gl_Texture = dynamic_cast<OpenGLBaseTexture*>(texture.get());
				gl_Texture->SetSamplerName(name);

				return;
			}
		}

		HZ_CORE_ERROR("Has no {0} texture in the shader {1}", name, m_Name)
	}

	Ref<Texture> OpenGLShader::GetTexture(const std::string& name) const
	{
		return GetTexture(name, 0);
	}

	Ref<Texture> OpenGLShader::GetTexture(const std::string& name, uint32_t index) const
	{
		auto textures = m_Textures.at(name);
		HZ_CORE_ASSERT(
			index < textures.size(),
			"Has no texture data in the index " + std::to_string(index) + " from the texture " + name
		)
		return textures[index];
	}

	std::vector<Ref<Texture>> OpenGLShader::GetTexturesVector() const
	{
		std::vector<Ref<Texture>> textures;
		for (auto it = m_Textures.begin(); it != m_Textures.end(); it++)
		{
			auto& textureArray = it->second;
			textures.insert(textures.end(), textureArray.begin(), textureArray.end());
		}
		return textures;
	}

	std::unordered_map<std::string, Ref<Texture>> OpenGLShader::GetTextures() const
	{
		std::unordered_map<std::string, Ref<Texture>> textures;
		for (auto it = m_Textures.begin(); it != m_Textures.end(); it++)
		{
			textures[it->first] = it->second[0];
		}
		return textures;
	}

	void OpenGLShader::SetMaterialUniformBuffer(Buffer buffer, uint32_t materialIndex)
	{
		if (HasVSMaterialUniformBuffer() || HasPSMaterialUniformBuffer())
		{
			auto gl_Buffer = dynamic_cast<OpenGLUniformBuffer*>(m_MaterialUniformBuffer.get());
			auto offset = materialIndex * m_MaterialUniformBufferAlignment;

			HZ_CORE_ASSERT(offset + buffer.Size <= gl_Buffer->GetSize(), "Material Uniform Buffer overflow!")

			byte* mapped = (byte*)m_MaterialUniformBuffer->Map();
			memcpy(mapped + offset, buffer.Data, buffer.Size);
			m_MaterialUniformBuffer->Unmap(offset, buffer.Size);
			return;
		}

		HZ_CORE_ERROR("Shader has no material uniform buffer!")
	}

	std::vector<Ref<UniformBuffer>> OpenGLShader::GetUniformBuffers() const
	{
		std::vector<Ref<UniformBuffer>> ubuffers(m_UniformBuffers.size());

		uint32_t index = 0;
		for (auto it = m_UniformBuffers.begin(); it != m_UniformBuffers.end(); it++)
		{
			ubuffers[index++] = it->second;
		}

		return ubuffers;
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const
	{
		GLint location = glGetUniformLocation(m_RendererId, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		HZ_PROFILE_FUNCTION()
		
		std::string result;
		std::ifstream in(filepath, std::ios::in, std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(result.data(), result.size());
			in.close();
		}
		else
		{
			HZ_CORE_ERROR("Could not open file '{0}'", filepath)
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		HZ_PROFILE_FUNCTION()
		
		std::unordered_map<GLenum, std::string> shaderSrcs;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			HZ_CORE_ASSERT(eol != std::string::npos, "Syntax error")
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			HZ_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified")

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSrcs[ShaderTypeFromString(type)] = source.substr(
				nextLinePos,
				pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos)
			);
		}

		return shaderSrcs;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSrcs)
	{
		HZ_PROFILE_FUNCTION()
		
		// Store shader ids
		std::vector<GLuint> shaderIds;

		// Get a program object.
		GLuint program = glCreateProgram();

		// Process each shader
		for (auto& kv : shaderSrcs)
		{
			GLenum type = kv.first;
			const std::string& source = kv.second;

			// Create an empty shader handle
			GLuint shader = glCreateShader(type);

			// Send the shader source code to GL
			const GLchar* src = (const GLchar*)source.c_str();
			glShaderSource(shader, 1, &src, 0);

			// Compile the shader
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				// We don't need the shader anymore
				glDeleteShader(shader);

				HZ_CORE_ERROR("{0}", infoLog.data())
				HZ_CORE_ASSERT(false, "Shader compilation failure!")
				return;
			}

			// Attach our shader to program
			glAttachShader(program, shader);

			// Store the shader id
			shaderIds.push_back(shader);
		}

		// Vertex and fragment shaders are successfully compiled.
		// Now time to link them together into a program.
		m_RendererId = program;

		// Link our program
		glLinkProgram(m_RendererId);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(m_RendererId, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(m_RendererId, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(m_RendererId, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore
			glDeleteProgram(m_RendererId);

			// Don't leak shaders either
			for (auto shaderId : shaderIds)
			{
				glDeleteShader(shaderId);
			}


			HZ_CORE_ERROR("{0}", infoLog.data())
			HZ_CORE_ASSERT(false, "Shader link failure!")
			return;
		}

		// Always detach shaders after a successful link
		for (auto shaderId : shaderIds)
		{
			glDetachShader(m_RendererId, shaderId);
		}
	}

	ShaderStruct* OpenGLShader::ParseShaderStruct(spirv_cross::Compiler& compiler, spirv_cross::SPIRType type, const std::string& name, ShaderDomain domain)
	{
		auto ustruct = new ShaderStruct(name);

		for (size_t i = 0; i < type.member_types.size(); i++)
		{
			auto fieldType = compiler.get_type(type.member_types[i]);
			auto fieldName = compiler.get_member_name(type.self, i);

			// Get type
			auto type = OpenGLShaderUniformDeclaration::Type::None;
			ShaderStruct* fieldStruct = nullptr;
			switch (fieldType.basetype)
			{
			case spirv_cross::SPIRType::Struct:
				fieldStruct = ParseShaderStruct(compiler, fieldType, fieldName, domain);
				break;

			case spirv_cross::SPIRType::Boolean:
				type = OpenGLShaderUniformDeclaration::Type::Boolean;
				break;

			case spirv_cross::SPIRType::Int:
				type = OpenGLShaderUniformDeclaration::Type::Int32;
				break;

			case spirv_cross::SPIRType::UInt:
				type = OpenGLShaderUniformDeclaration::Type::UInt32;
				break;

			case spirv_cross::SPIRType::Float:
				if (fieldType.vecsize == 1)
					type = OpenGLShaderUniformDeclaration::Type::Float32;
				else if (fieldType.vecsize == 2)
					type = OpenGLShaderUniformDeclaration::Type::Vec2;
				else if (fieldType.vecsize == 3)
					type = OpenGLShaderUniformDeclaration::Type::Vec3;
				else if (fieldType.vecsize == 2)
					type = OpenGLShaderUniformDeclaration::Type::Vec2;
				else if (fieldType.vecsize == 4 && fieldType.columns == 1)
					type = OpenGLShaderUniformDeclaration::Type::Vec4;
				else if (fieldType.vecsize == 4 && fieldType.columns == 3)
					type = OpenGLShaderUniformDeclaration::Type::Mat3;
				else if (fieldType.vecsize == 4 && fieldType.columns == 4)
					type = OpenGLShaderUniformDeclaration::Type::Mat4;
				break;
			}

			// Has at least 1 uniform
			uint32_t count = 1;

			// Check if is an array and his size
			if (fieldType.array.size() > 0)
			{
				// Support only vectors now
				count = fieldType.array[0];
			}

			OpenGLShaderUniformDeclaration* field;
			if (fieldStruct)
				field = new OpenGLShaderUniformDeclaration(fieldName, domain, fieldStruct, count);
			else
				field = new OpenGLShaderUniformDeclaration(fieldName, domain, type, count);

			ustruct->AddField(field);
		}

		return ustruct;
	}

	void OpenGLShader::GetShaderResources(const std::vector<uint32_t>& spirv, ShaderDomain domain)
	{
		spirv_cross::Compiler compiler(spirv);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		// samplerCube, sampler2D
		for (const auto& image : resources.sampled_images)
		{
			unsigned binding = compiler.get_decoration(image.id, spv::DecorationBinding);

			// Has at least 1 texture
			uint32_t count = 1;

			// Check if is an array and his size
			const auto& spvType = compiler.get_type(image.type_id);
			if (spvType.array.size() > 0)
			{
				count = 0;

				for (uint32_t i = 0; i < spvType.array.size(); i++)
				{
					count += spvType.array[i];
				}
			}

			// Type here is SampledImage
			auto type = OpenGLShaderResourceDeclaration::Type::SampledImage;

			// Try to get the dimension of this texture
			auto dimension = OpenGLShaderResourceDeclaration::Dimension::None;
			const auto& spvBaseType = compiler.get_type(image.base_type_id);
			switch (spvBaseType.image.dim)
			{
			case spv::Dim2D:	dimension = OpenGLShaderResourceDeclaration::Dimension::Texture2D;   break;
			case spv::DimCube:	dimension = OpenGLShaderResourceDeclaration::Dimension::TextureCube; break;
			}

			// Create the Resource Declaration
			auto resourceDecl = new OpenGLShaderResourceDeclaration(image.name, binding, domain, type, dimension, count);
			m_Resources.push_back(resourceDecl);
		}

		// texture2D
		for (const auto& image : resources.separate_images)
		{
			unsigned binding = compiler.get_decoration(image.id, spv::DecorationBinding);

			// Has at least 1 resource
			uint32_t count = 1;

			// Check if is an array and his size
			const auto& spvType = compiler.get_type(image.type_id);
			if (spvType.array.size() > 0)
			{
				count = 0;

				for (uint32_t i = 0; i < spvType.array.size(); i++)
				{
					count += spvType.array[i];
				}
			}

			// Type here is Image
			auto type = OpenGLShaderResourceDeclaration::Type::Image;

			// Try to get the dimension of this texture
			auto dimension = OpenGLShaderResourceDeclaration::Dimension::None;
			const auto& spvBaseType = compiler.get_type(image.base_type_id);
			switch (spvBaseType.image.dim)
			{
			case spv::Dim2D:	dimension = OpenGLShaderResourceDeclaration::Dimension::Texture2D;   break;
			case spv::DimCube:	dimension = OpenGLShaderResourceDeclaration::Dimension::TextureCube; break;
			}

			// Create the Resource Declaration
			auto resourceDecl = new OpenGLShaderResourceDeclaration(image.name, binding, domain, type, dimension, count);
			m_Resources.push_back(resourceDecl);
		}

		// sampler
		for (const auto& sampler : resources.separate_samplers)
		{
			unsigned binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);

			// Has at least 1 resource
			uint32_t count = 1;

			// Check if is an array and his size
			const auto& spvType = compiler.get_type(sampler.type_id);
			if (spvType.array.size() > 0)
			{
				count = 0;

				for (uint32_t i = 0; i < spvType.array.size(); i++)
				{
					count += spvType.array[i];
				}
			}

			// Type here is Sampler
			auto type = OpenGLShaderResourceDeclaration::Type::Sampler;

			// Sampler has no dimension
			auto dimension = OpenGLShaderResourceDeclaration::Dimension::None;

			// Create the Resource Declaration
			auto resourceDecl = new OpenGLShaderResourceDeclaration(sampler.name, binding, domain, type, dimension, count);
			m_Resources.push_back(resourceDecl);
		}

		// uniform buffer
		for (const auto& ubuffer : resources.uniform_buffers)
		{
			unsigned binding = compiler.get_decoration(ubuffer.id, spv::DecorationBinding);

			// Uses the name queried using the id, as it returns the variable name rather than block name
			auto name = compiler.get_name(ubuffer.id);

			// Create the Uniform Buffer Declaration
			auto bufferDecl = CreateScope<OpenGLShaderUniformBufferDeclaration>(name, binding, domain);

			// Loop through all members of the uniform buffer, and create a Uniform Declaration for each them
			const auto& spvBaseType = compiler.get_type(ubuffer.base_type_id);
			for (size_t i = 0; i < spvBaseType.member_types.size(); i++)
			{
				auto memberType = compiler.get_type(spvBaseType.member_types[i]);
				auto memberName = compiler.get_member_name(spvBaseType.self, i);

				// Get type
				auto type = OpenGLShaderUniformDeclaration::Type::None;
				ShaderStruct* ustruct = nullptr;
				switch (memberType.basetype)
				{
				case spirv_cross::SPIRType::Struct:
					ustruct = ParseShaderStruct(compiler, memberType, memberName, domain);
					break;

				case spirv_cross::SPIRType::Boolean:
					type = OpenGLShaderUniformDeclaration::Type::Boolean;
					break;

				case spirv_cross::SPIRType::Int:
					type = OpenGLShaderUniformDeclaration::Type::Int32;
					break;

				case spirv_cross::SPIRType::UInt:
					type = OpenGLShaderUniformDeclaration::Type::UInt32;
					break;

				case spirv_cross::SPIRType::Float:
					if (memberType.vecsize == 1)
						type = OpenGLShaderUniformDeclaration::Type::Float32;
					else if (memberType.vecsize == 2)
						type = OpenGLShaderUniformDeclaration::Type::Vec2;
					else if (memberType.vecsize == 3)
						type = OpenGLShaderUniformDeclaration::Type::Vec3;
					else if (memberType.vecsize == 2)
						type = OpenGLShaderUniformDeclaration::Type::Vec2;
					else if (memberType.vecsize == 4 && memberType.columns == 1)
						type = OpenGLShaderUniformDeclaration::Type::Vec4;
					else if (memberType.vecsize == 4 && memberType.columns == 3)
						type = OpenGLShaderUniformDeclaration::Type::Mat3;
					else if (memberType.vecsize == 4 && memberType.columns == 4)
						type = OpenGLShaderUniformDeclaration::Type::Mat4;
					break;
				}

				// Has at least 1 uniform
				uint32_t count = 1;

				// Check if is an array and his size
				if (memberType.array.size() > 0)
				{
					// Support only vectors now
					count = memberType.array[0];
				}

				OpenGLShaderUniformDeclaration* uniform;
				if (ustruct)
					uniform = new OpenGLShaderUniformDeclaration(memberName, domain, ustruct, count);
				else
					uniform = new OpenGLShaderUniformDeclaration(memberName, domain, type, count);

				bufferDecl->PushUniform(uniform);
			}

			// TODO: Define this string as static
			if (ubuffer.name == "MaterialData")
			{
				switch (domain)
				{
				case Hazel::ShaderDomain::Vertex:
					m_VSMaterialUniformBuffer = std::move(bufferDecl);
					break;

				case Hazel::ShaderDomain::Pixel:
					m_PSMaterialUniformBuffer = std::move(bufferDecl);
					break;
				}
			}
			else
			{
				auto decl = bufferDecl.release();
				switch (domain)
				{
					case Hazel::ShaderDomain::Vertex: m_VSUniformBuffers.push_back(decl); break;
					case Hazel::ShaderDomain::Pixel:  m_PSUniformBuffers.push_back(decl); break;
				}
			}
		}
	}

	void OpenGLShader::GenerateShaderResources()
	{
		// VS Uniform Buffers
		for (auto decl : m_VSUniformBuffers)
		{
			auto ubuffer = UniformBuffer::Create(decl->GetSize());

			// Set the binding, needed for OpenGL rendering
			auto gl_UBuffer = static_cast<OpenGLUniformBuffer*>(ubuffer.get());
			gl_UBuffer->SetBinding(decl->GetBinding());

			m_UniformBuffers[decl->GetName()] = ubuffer;
		}

		// PS Uniform Buffers
		for (auto decl : m_PSUniformBuffers)
		{
			auto ubuffer = UniformBuffer::Create(decl->GetSize());

			// Set the binding, needed for OpenGL rendering
			auto gl_UBuffer = static_cast<OpenGLUniformBuffer*>(ubuffer.get());
			gl_UBuffer->SetBinding(decl->GetBinding());

			m_UniformBuffers[decl->GetName()] = ubuffer;
		}

		// Material Uniform Buffer
		if (HasVSMaterialUniformBuffer() || HasPSMaterialUniformBuffer())
		{
			ShaderUniformBufferDeclaration* ubuffer;
			if (HasVSMaterialUniformBuffer())
				ubuffer = m_VSMaterialUniformBuffer.get();
			else
				ubuffer = m_PSMaterialUniformBuffer.get();

			int32_t minUniformBufferAlignment = 0;
			glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &minUniformBufferAlignment);

			m_MaterialUniformBufferAlignment = ubuffer->GetSize();
			if (minUniformBufferAlignment > 0)
			{
				m_MaterialUniformBufferAlignment =
					(m_MaterialUniformBufferAlignment + minUniformBufferAlignment - 1) &
					~(minUniformBufferAlignment - 1);
			}

			m_MaterialUniformBuffer = CreateScope<OpenGLUniformBuffer>(m_MaterialUniformBufferAlignment * MAX_MAT_INSTANCES);

			// Set the binding, needed for OpenGL rendering
			auto gl_UBuffer = dynamic_cast<OpenGLUniformBuffer*>(m_MaterialUniformBuffer.get());
			gl_UBuffer->SetBinding(ubuffer->GetBinding());
		}

		// Textures
		for (auto resource : m_Resources)
		{
			auto gl_Resource = dynamic_cast<OpenGLShaderResourceDeclaration*>(resource);
			
			// Only process Images and SampledImages
			if (gl_Resource->GetType() == OpenGLShaderResourceDeclaration::Type::Image ||
				gl_Resource->GetType() == OpenGLShaderResourceDeclaration::Type::SampledImage)
			{
				Ref<Texture> defaultTex;

				if (gl_Resource->GetDimension() == OpenGLShaderResourceDeclaration::Dimension::Texture2D)
				{
					// Set to default texture (1x1 magenta color)
					defaultTex = Texture2D::Create(TextureFormat::RGB, 1, 1);
				}
				else if (gl_Resource->GetDimension() == OpenGLShaderResourceDeclaration::Dimension::TextureCube)
				{
					// Set to default cubemap texture (6 sides, 4x4 rand color each side)
					defaultTex = TextureCube::Create(TextureFormat::RGBA, 4, 4);
				}

				std::vector<Ref<Texture>> textures(resource->GetCount());
				
				for (uint32_t i = 0; i < resource->GetCount(); i++)
				{
					textures[i] = defaultTex;

					// Set the sampler name, so the sampler int will be set correctly
					auto gl_Texture = dynamic_cast<OpenGLBaseTexture*>(textures[i].get());
					gl_Texture->SetSamplerName(resource->GetName() + "[" + std::to_string(i) + "]");
				}

				m_Textures[resource->GetName()] = textures;
			}
		}
	}

}