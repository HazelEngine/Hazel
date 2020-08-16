#pragma once

#include <Hazel/Core/Core.h>
#include <Hazel/Core/Buffer.h>
#include <Hazel/Renderer/RendererAPI.h>

namespace Hazel {

	enum class TextureFormat
	{
		None		= 0,
		RGB			= 1,
		RGBA		= 2,
		Float16		= 3
	};

	enum class TextureWrap
	{
		None		= 0,
		Clamp		= 1,
		Repeat		= 2
	};

	class HAZEL_API Texture
	{
	public:
		virtual ~Texture() = default;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual TextureFormat GetFormat() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetMipLevelCount() const = 0;

		virtual RendererId GetRendererId() const = 0;

	public:
		static uint32_t GetBPP(TextureFormat format);
		static uint32_t CalculateMipMapCount(uint32_t width, uint32_t height);

	public:
		virtual bool operator==(const Texture& other) const = 0;
	};

	class HAZEL_API Texture2D : public Texture
	{
	public:
		virtual void Lock() = 0;
		virtual void Unlock() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual Buffer GetWriteableBuffer() = 0;

		virtual bool Loaded() const = 0;

		virtual const std::string& GetPath() const = 0;

	public:
		static Ref<Texture2D> Create(
			TextureFormat format,
			uint32_t width,
			uint32_t height,
			TextureWrap wrap = TextureWrap::Clamp
		);

		static Ref<Texture2D> Create(
			TextureFormat format,
			uint32_t width,
			uint32_t height,
			void* data,
			TextureWrap wrap = TextureWrap::Clamp
		);

		static Ref<Texture2D> Create(const std::string& path, bool srgb = false);
	};

	class HAZEL_API TextureCube : public Texture
	{
	public:
		virtual const std::string& GetPath() const = 0;

	public:
		static Ref<TextureCube> Create(TextureFormat format, uint32_t width, uint32_t height);
		static Ref<TextureCube> Create(const std::string& path);
	};

}