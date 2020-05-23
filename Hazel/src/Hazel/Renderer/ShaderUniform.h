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
		friend class ShaderStruct;

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

	class ShaderStruct
	{
		friend class Shader;

	public:
		ShaderStruct(const std::string& name)
			: m_Name(name), m_Size(0), m_Offset(0) {}

		void AddField(ShaderUniformDeclaration* field)
		{
			m_Size += field->GetSize();
			
			uint32_t offset = 0;
			if (m_Fields.size() > 0)
			{
				ShaderUniformDeclaration* previous = m_Fields.back();
				offset = previous->GetOffset() + previous->GetSize();
			}
			field->SetOffset(offset);

			m_Fields.push_back(field);
		}

		inline void SetOffset(uint32_t offset) { m_Offset = offset; }

		inline const std::string& GetName() const { return m_Name; }
		inline uint32_t GetSize() const { return m_Size; }
		inline uint32_t GetOffset() const { return m_Offset; }
		inline const ShaderUniformList& GetFields() const { return m_Fields; }

	private:
		std::string m_Name;
		ShaderUniformList m_Fields;
		uint32_t m_Size;
		uint32_t m_Offset;
	};

}