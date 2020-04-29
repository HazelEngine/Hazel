#pragma once

#include <Hazel/Core/Core.h>
#include <Hazel/Renderer/Buffer.h>
#include <Hazel/Renderer/Shader.h>

namespace Hazel {

	struct PipelineSpecification
	{
		Ref<Shader> Shader;
		BufferLayout VertexBufferLayout;
	};
	
	class HAZEL_API Pipeline
	{
	public:
		virtual ~Pipeline() = default;

		virtual const PipelineSpecification& GetSpecification() const = 0;

		static Ref<Pipeline> Create(const PipelineSpecification& spec);
	};

}