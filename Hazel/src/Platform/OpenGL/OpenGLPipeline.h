#pragma once

#include <Hazel/Renderer/Pipeline.h>

namespace Hazel {

	class HAZEL_API OpenGLPipeline : public Pipeline
	{
	public:
		OpenGLPipeline(const PipelineSpecification& spec);
		virtual ~OpenGLPipeline() = default;

		virtual const PipelineSpecification& GetSpecification() const { return m_Specification; }

		virtual uint32_t GetVertexArrayRendererId() const { return m_VertexArrayRendererId; }

	private:
		PipelineSpecification m_Specification;

		uint32_t m_VertexArrayRendererId;
	};

}