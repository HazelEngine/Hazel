#pragma once

#include <string>

#include "Hazel/Core/Core.h"

namespace Hazel {

	class HAZEL_API Texture
	{
	public:
		virtual ~Texture() = default;

		virtual void Bind(uint32_t slot) const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
	};

	class HAZEL_API Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::string& path);
		
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);

		static Ref<Texture2D> Create(
			const void* data,
			uint32_t width,
			uint32_t height,
			uint32_t channels = 3
		);
	};

}