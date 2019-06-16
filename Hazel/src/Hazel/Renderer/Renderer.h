#pragma once

namespace Hazel {

	enum class RendererAPI
	{
		None = 0,
		OpenGL = 1
	};

	class HAZEL_API Renderer
	{
	public:
		inline static RendererAPI GetAPI() { return s_RendererAPI; }

	private:
		static RendererAPI s_RendererAPI;
	};

}