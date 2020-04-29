#include "hzpch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Hazel {

	namespace {
	
		std::vector<uint32_t> LoadSpirvFile(const std::string& path)
	{
		FILE* file;
		fopen_s(&file, path.c_str(), "rb");
		if (!file)
		{
			HZ_CORE_ASSERT(false, "Failed to open SPIR-V file.")
			return {};
		}

		fseek(file, 0, SEEK_END);
		long len = ftell(file) / sizeof(uint32_t);
		rewind(file);

		std::vector<uint32_t> spirv(len);
		if (fread(spirv.data(), sizeof(uint32_t), len, file) != size_t(len))
			spirv.clear();

		fclose(file);
		return spirv;
	}

	}

	Ref<Shader> Shader::Create(const ShaderCreateInfo& info)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
				return nullptr;

			case RendererAPI::API::OpenGL:	
				return CreateRef<OpenGLShader>(info);

			case RendererAPI::API::Vulkan:	
				return CreateRef<VulkanShader>(info);
		}

		HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}

	Ref<Shader> Shader::Create(
		const std::string& name,
		const std::string& vsPath,
		const std::string& fsPath
	) {
		ShaderCreateInfo info = {};
		info.Name = name;
		info.VertexShaderSource = LoadSpirvFile(vsPath);
		info.FragmentShaderSource = LoadSpirvFile(fsPath);
		return Create(info);
	}

}