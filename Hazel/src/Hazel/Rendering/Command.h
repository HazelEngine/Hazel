#pragma once

#include "Hazel/Core.h"

namespace Hazel {

	// Forward declarations:
	class Renderer;
	class RenderPass;

	class HAZEL_API Command
	{
	public:
		//! CTOR/DTOR:
		Command(RenderPass* parent);
		virtual ~Command();

		//! VIRTUALS:
		virtual void Build() = 0;
		virtual void Viewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) = 0;
		virtual void Scissor(int x, int y, int width, int height) = 0;
		virtual void Test(unsigned int offset) {}

		virtual void Draw(
			uint32_t vertexCount,
			uint32_t instanceCount = 1,
			uint32_t firstVertex = 0,
			uint32_t firstInstance = 0
		) = 0;

		virtual void DrawIndexed(
			uint32_t indexCount,
			uint32_t instanceCount = 1,
			uint32_t firstIndex = 0,
			int32_t  vertexOffset = 0,
			uint32_t firstInstance = 0
		) = 0;

		virtual void Cleanup() = 0;

		//! ACCESSORS:
		RenderPass* GetParent() const;

	protected:
		//! MEMBERS:
		RenderPass* m_Parent;
	};

	////////////////////////////////////////////////////////////////////////////////
	// Command inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline Command::Command(RenderPass* parent) :
		m_Parent(parent)
	{
	}

	inline Command::~Command()
	{
	}

	inline RenderPass* Command::GetParent() const
	{
		return m_Parent;
	}

}