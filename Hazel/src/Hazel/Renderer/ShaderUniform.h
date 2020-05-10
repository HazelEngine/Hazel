#pragma once

#include <Hazel/Core/Core.h>
#include <Hazel/Core/Log.h>

#include <string>
#include <vector>

namespace Hazel {

	enum class ShaderDomain
	{
		None = 0, Vertex = 1, Pixel = 2
	};

	class ShaderResourceDeclaration
	{
	public:
		virtual const std::string& GetName() const = 0;
		virtual uint32_t GetBinding() const = 0;
		virtual uint32_t GetCount() const = 0;
		virtual ShaderDomain GetDomain() const = 0;
	};

	typedef std::vector<ShaderResourceDeclaration*> ShaderResourceList;

	class ShaderUniformDeclaration
	{
		friend class Shader;

	public:
		virtual const std::string& GetName() const = 0;
		virtual uint32_t GetSize() const = 0;
		virtual uint32_t GetCount() const = 0;
		virtual uint32_t GetOffset() const = 0;
		virtual ShaderDomain GetDomain() const = 0;

	protected:
		virtual void SetOffset(uint32_t offset) = 0;
	};

	typedef std::vector<ShaderUniformDeclaration*> ShaderUniformList;

	class ShaderUniformBufferDeclaration
	{
	public:
		virtual const std::string& GetName() const = 0;
		virtual uint32_t GetBinding() const = 0;
		virtual uint32_t GetSize() const = 0;
		virtual const ShaderUniformList& GetUniformDeclarations() const = 0;
		virtual ShaderDomain GetDomain() const = 0;

		virtual ShaderUniformDeclaration* FindUniform(const std::string& name) = 0;
	};

	typedef std::vector<ShaderUniformBufferDeclaration*> ShaderUniformBufferList;

}