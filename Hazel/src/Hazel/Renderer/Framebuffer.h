#pragma once

#include <Hazel/Core/Core.h>

#include <glm/glm.hpp>

namespace Hazel {

	struct FramebufferSpecification
	{
		glm::vec4 ClearColor;
	};

	class HAZEL_API Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};

}