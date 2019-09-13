#include "hzpch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

// TODO: Remove!
#include <spirv_glsl.hpp>

namespace Hazel {

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

	Shader* Shader::Create(const std::string& filepath)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
				return nullptr;

			case RendererAPI::API::OpenGL:	
				return new OpenGLShader(filepath);
		}

		HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}

	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
				return nullptr;

			case RendererAPI::API::OpenGL:	
				return new OpenGLShader(vertexSrc, fragmentSrc);
		}

		HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}

	// TODO: Put the implementation inside Platform!
	Shader* Shader::CreateFromSpirv(const std::string& vsPath, const std::string& fsPath)
	{
		spirv_cross::CompilerGLSL::Options options;
		options.version = 330;
		options.es = false;
		options.emit_uniform_buffer_as_plain_uniforms = true;

		std::vector<uint32_t> vs = LoadSpirvFile(vsPath);
		std::vector<uint32_t> fs = LoadSpirvFile(fsPath);

		spirv_cross::CompilerGLSL vsCompiler(std::move(vs));
		spirv_cross::CompilerGLSL fsCompiler(std::move(fs));

		vsCompiler.set_common_options(options);
		fsCompiler.set_common_options(options);

		std::string vsSource, fsSource;
		vsSource = vsCompiler.compile();
		fsSource = fsCompiler.compile();

		return Create(vsSource, fsSource);
	}

}