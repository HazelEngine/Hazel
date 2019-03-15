#pragma once

#include "Hazel/Rendering/Renderer.h"

// Forward declarations:
struct IDXGIFactory4;
struct IDXGIAdapter1;
struct ID3D12Device;
struct ID3D12CommandQueue;
struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;
struct IDXGISwapChain3;
struct ID3D12Resource;
struct ID3D12Fence;
typedef void* HANDLE;
enum DXGI_FORMAT;

namespace Hazel {

	class HAZEL_API D3D12Renderer : public Renderer
	{
	public:
		friend class D3D12Buffer;
		friend class D3D12Texture;
		friend class D3D12Command;
		friend class D3D12RenderPass;

		//! CTOR/DTOR:
		D3D12Renderer(const Window& window, HWND handler, BufferingType buffering = eTriple);
		virtual ~D3D12Renderer();

		////////////////////////////////////////////////////////////////////////
		// D3D12Renderer::CreateBuffer:
		////////////////////////////////////////////////////////////////////////

		std::shared_ptr<Buffer> CreateBuffer(
			uint32_t size,
			const void* data,
			const Buffer::CreateInfo& ci
		) override;

		////////////////////////////////////////////////////////////////////////
		// D3D12Renderer::CreateTexture:
		////////////////////////////////////////////////////////////////////////

		std::shared_ptr<Texture> CreateTexture(
			uint32_t width,
			uint32_t height,
			uint32_t depth,
			const void* data,
			const Texture::CreateInfo& ci
		) override;

		////////////////////////////////////////////////////////////////////////
		// D3D12Renderer::CreateRenderPass:
		////////////////////////////////////////////////////////////////////////

		RenderPass* CreateRenderPass(
			RenderPass* parent,
			const RenderPass::CreateInfo& ci
		) override;

		void Initialize() override;
		void Resize(int width, int height) override;
		void Display() override;
		void Cleanup() override;
		Type GetType() const override;

		//! SERVICES:
		static DXGI_FORMAT ConvertToDxFormat(Format format);

	protected:
		struct SwapchainImage
		{
			ID3D12Resource* View;
			ID3D12Fence* Fence;
			uint64_t FenceValue;
		};

		//! SERVICES:
		void CreateDevice();
		void CreateCommandQueue();
		void CreateSwapchain();

		//! MEMBERS:
		IDXGIFactory4* m_Factory;
		IDXGIAdapter1* m_Gpu;
		ID3D12Device* m_Device;
		ID3D12CommandQueue* m_CommandQueue;
		ID3D12DescriptorHeap* m_SwapchainHeap;
		IDXGISwapChain3* m_Swapchain;
		std::vector<SwapchainImage> m_SwapchainImages;
		HANDLE m_FenceEvent;

		HWND m_Handler;
	};

	////////////////////////////////////////////////////////////////////////////////
	// D3D12Renderer inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline D3D12Renderer::D3D12Renderer(const Window& window, HWND handler, BufferingType buffering) :
		Renderer(window, buffering),
		m_Factory(nullptr),
		m_Gpu(nullptr),
		m_Device(nullptr),
		m_CommandQueue(nullptr),
		m_SwapchainHeap(nullptr),
		m_Swapchain(nullptr),
		m_FenceEvent(nullptr),
		m_Handler(handler)
	{
	}

	inline D3D12Renderer::~D3D12Renderer()
	{
	}

	inline Renderer::Type D3D12Renderer::GetType() const
	{
		return eD3D12;
	}

}