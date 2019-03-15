#pragma once

#include "Hazel/Core.h"
#include "Hazel/Window.h"

#include "Hazel/Rendering/Buffer.h"
#include "Hazel/Rendering/Texture.h"
#include "Hazel/Rendering/Command.h"
#include "Hazel/Rendering/RenderPass.h"

namespace Hazel {

	// Forward declarations:
	class Window;

	class HAZEL_API Renderer
	{
	public:
		//! TYPEDEF/ENUMS:
		enum Type
		{
			eUndefined,
			eD3D12,
			eVulkan
		};

		enum BufferingType
		{
			eSingle,
			eDouble,
			eTriple
		};

		struct GpuInfo
		{
			//! SERVICES:
			std::string Print() const;

			//! MEMBERS:
			uint32_t VendorId;
			uint32_t DeviceId;
			std::string Info;
		};

		//! CTOR/DTOR:
		Renderer(const Window& window, BufferingType buffering = eTriple);
		virtual ~Renderer();

		////////////////////////////////////////////////////////////////////////
		// Renderer::CreateBuffer:
		////////////////////////////////////////////////////////////////////////

		virtual std::shared_ptr<Buffer> CreateBuffer(
			uint32_t size,
			const void* data,
			const Buffer::CreateInfo& ci = Buffer::CreateInfo::GetDefault()
		) = 0;

		////////////////////////////////////////////////////////////////////////
		// Renderer::CreateTexture:
		////////////////////////////////////////////////////////////////////////

		virtual std::shared_ptr<Texture> CreateTexture(
			uint32_t width,
			uint32_t height,
			uint32_t depth,
			const void* data,
			const Texture::CreateInfo& ci = Texture::CreateInfo::GetDefault()
		) = 0;

		////////////////////////////////////////////////////////////////////////
		// Renderer::CreateRenderPass:
		////////////////////////////////////////////////////////////////////////

		virtual RenderPass* CreateRenderPass(
			RenderPass* parent,
			const RenderPass::CreateInfo& ci
		) = 0;

		virtual void Initialize() = 0;
		virtual void Resize(int width, int height) = 0;
		virtual void Display() = 0;
		virtual void Cleanup() = 0;
		virtual Type GetType() const;

		//! ACCESSORS:
		const Window& GetWindow() const;
		uint32_t GetBufferCount() const;
		uint32_t GetBufferIndex() const;
		const GpuInfo& GetGpuInfo() const;

	protected:
		//! MEMBERS:
		const Window& m_Window;
		std::vector<std::shared_ptr<RenderPass>> m_RenderPasses;
		BufferingType m_BufferingType;
		uint32_t m_BufferIndex;
		GpuInfo m_GpuInfo;
	};

	////////////////////////////////////////////////////////////////////////////////
	// Renderer::GpuInfo inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline std::string Renderer::GpuInfo::Print() const
	{
		return (
			"VID = " + std::to_string(VendorId) + " " +
			"PID = " + std::to_string(DeviceId) + " / " + Info
		);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Renderer inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline Renderer::Renderer(const Window& window, BufferingType buffering) :
		m_Window(window),
		m_BufferingType(buffering),
		m_BufferIndex(0),
		m_GpuInfo({ 0, 0, "Undefined" })
	{
	}

	inline Renderer::~Renderer()
	{
	}

	inline Renderer::Type Renderer::GetType() const
	{
		return eUndefined;
	}

	inline const Window& Renderer::GetWindow() const
	{
		return m_Window;
	}

	inline uint32_t Renderer::GetBufferCount() const
	{
		switch (m_BufferingType)
		{
		case eSingle: return 1;
		case eDouble: return 2;
		case eTriple: return 3;
		default: break;
		}

		return 0;
	}

	inline uint32_t Renderer::GetBufferIndex() const
	{
		return m_BufferIndex;
	}

	inline const Renderer::GpuInfo& Renderer::GetGpuInfo() const
	{
		return m_GpuInfo;
	}
}