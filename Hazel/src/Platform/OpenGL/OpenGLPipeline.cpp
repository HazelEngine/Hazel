#include "hzpch.h"
#include "OpenGLPipeline.h"

#include <glad/glad.h>

namespace Hazel {

	OpenGLPipeline::OpenGLPipeline(const PipelineSpecification& spec)
		: m_Specification(spec)
	{
		glGenVertexArrays(1, &m_VertexArrayRendererId);
	}

}