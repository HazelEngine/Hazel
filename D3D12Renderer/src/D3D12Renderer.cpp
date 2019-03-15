#include "hzpch.h"
#include "D3D12Renderer.h"

#include "D3D12Buffer.h"
#include "D3D12Texture.h"
#include "D3D12Command.h"
#include "D3D12RenderPass.h"

/* TODO: Remove this! */
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Hazel {

	std::shared_ptr<Hazel::Buffer> D3D12Renderer::CreateBuffer(
		uint32_t size,
		const void* data,
		const Buffer::CreateInfo& ci)
	{
		auto buffer = std::shared_ptr<Buffer>(new D3D12Buffer(this));
		buffer->Build(size, data, ci);
		return buffer;
	}

	std::shared_ptr<Hazel::Texture> D3D12Renderer::CreateTexture(
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		const void* data,
		const Texture::CreateInfo& ci)
	{
		auto texture = std::shared_ptr<Texture>(new D3D12Texture(this, width, height, depth));
		texture->Build(data, ci);
		return texture;
	}

	Hazel::RenderPass* D3D12Renderer::CreateRenderPass(
		RenderPass* parent,
		const RenderPass::CreateInfo& ci)
	{
		RenderPass* actualParent = nullptr;
		if (!m_RenderPasses.empty())
		{
			actualParent = m_RenderPasses.back().get();
		}

		auto renderPass = std::shared_ptr<RenderPass>(new D3D12RenderPass(this, actualParent));
		renderPass->Build(ci);

		if (ci.Type_ == RenderPass::eDrawOnce ||
			ci.Type_ == RenderPass::eDrawOnceBackground)
		{
			auto it = m_RenderPasses.begin();
			while (it != m_RenderPasses.end())
			{
				if ((*it)->GetType() == RenderPass::eDrawOnce ||
					(*it)->GetType() == RenderPass::eDrawOnceBackground)
				{
					it++;
				}
				else
				{
					break;
				}
			}

			m_RenderPasses.insert(it, renderPass);
		}
		else
		{
			m_RenderPasses.push_back(renderPass);
		}

		for (unsigned int i = 0; i < m_RenderPasses.size(); ++i)
		{
			auto renderPass = static_cast<D3D12RenderPass*>(m_RenderPasses[i].get());
			if (i > 0)
			{
				renderPass->m_Parent = m_RenderPasses[i - 1].get();
			}
			else
			{
				renderPass->m_Parent = nullptr;
			}
		}

		return renderPass.get();
	}

	void D3D12Renderer::Initialize()
	{
		D3D12Renderer::CreateDevice();
		D3D12Renderer::CreateCommandQueue();
		D3D12Renderer::CreateSwapchain();

		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_FenceEvent == NULL)
		{
			return;
		}
	}

	void D3D12Renderer::Resize(int width, int height)
	{
		if (!m_SwapchainImages.empty())
		{
			for (auto& swapchainImage : m_SwapchainImages)
			{
				swapchainImage.View->Release();
				swapchainImage.Fence->Release();
			}

			DXGI_SWAP_CHAIN_DESC desc = {};
			m_Swapchain->GetDesc(&desc);
			m_Swapchain->ResizeBuffers(
				Renderer::GetBufferCount(),
				width,
				height,
				desc.BufferDesc.Format,
				desc.Flags
			);

			m_SwapchainImages.clear();
		}

		CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_SwapchainHeap->GetCPUDescriptorHandleForHeapStart());
		UINT descriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_SwapchainImages.resize(Renderer::GetBufferCount());

		for (uint32_t i = 0; i < Renderer::GetBufferCount(); ++i)
		{
			if (FAILED(m_Swapchain->GetBuffer(
				i,
				__uuidof(ID3D12Resource), (void**)&m_SwapchainImages[i].View
			)))
			{
				//HZ_ERROR("Failed to get swapchain image buffer {0} of {1}!", i, Renderer::GetBufferCount())
				return;
			}

			if (FAILED(m_Device->CreateFence(
				0,
				D3D12_FENCE_FLAG_NONE,
				__uuidof(ID3D12Fence), (void**)&m_SwapchainImages[i].Fence
			)))
			{
				//HZ_ERROR("Failed to create swapchain image fence {0} of {1}!", i, Renderer::GetBufferCount())
				return;
			}

			m_Device->CreateRenderTargetView(
				m_SwapchainImages[i].View,
				NULL,
				descriptorHandle
			);

			descriptorHandle.Offset(1, descriptorSize);
			m_SwapchainImages[i].FenceValue = 0;
		}

		for (auto& renderPass : m_RenderPasses)
		{
			renderPass->Resize(width, height);
		}
	}

	void D3D12Renderer::Display()
	{
		m_BufferIndex = m_Swapchain->GetCurrentBackBufferIndex();
		auto& swapchainImage = m_SwapchainImages[m_BufferIndex];

		if (swapchainImage.Fence->GetCompletedValue() < swapchainImage.FenceValue)
		{
			swapchainImage.Fence->SetEventOnCompletion(
				swapchainImage.FenceValue,
				m_FenceEvent
			);

			WaitForSingleObject(m_FenceEvent, INFINITE);
		}

		for (auto& renderPass : m_RenderPasses)
		{
			renderPass->Record();
		}

		swapchainImage.FenceValue++;
		m_CommandQueue->Signal(swapchainImage.Fence, swapchainImage.FenceValue);
		m_Swapchain->Present(0, 0);
	}

	void D3D12Renderer::Cleanup()
	{
	}

	DXGI_FORMAT D3D12Renderer::ConvertToDxFormat(Format format)
	{
		auto FormatToDxFormat = std::map<Format, DXGI_FORMAT>() =
		{
			{ Format::eUndefined  , DXGI_FORMAT_UNKNOWN            },
			{ Format::eR32F       , DXGI_FORMAT_R32_FLOAT          },
			{ Format::eRG32F      , DXGI_FORMAT_R32G32_FLOAT       },
			{ Format::eRGB32F     , DXGI_FORMAT_R32G32B32_FLOAT    },
			{ Format::eRGBA32F    , DXGI_FORMAT_R32G32B32A32_FLOAT },
			{ Format::eR8         , DXGI_FORMAT_R8_UNORM           },
			{ Format::eRG8        , DXGI_FORMAT_R8G8_UNORM         },
			{ Format::eRGBA8      , DXGI_FORMAT_R8G8B8A8_UNORM     }
		};

		return FormatToDxFormat[format];
	}

	void D3D12Renderer::CreateDevice()
	{
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;
		UINT factoryFlags = 0;
		UINT adapterIndex = 0;

#ifdef HZ_DEBUG
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (FAILED(D3D12GetDebugInterface(
			IID_PPV_ARGS(&debugController)
		)))
		{
			//HZ_ERROR("Failed to get D3D12 debug interface!")
			return;
		}

		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		debugController->EnableDebugLayer();
#endif // HZ_DEBUG

		if (FAILED(CreateDXGIFactory2(
			factoryFlags,
			__uuidof(IDXGIFactory4), (void**)&m_Factory
		)))
		{
			//HZ_ERROR("Failed to create DXGI factory!")
			return;
		}

		while (m_Factory->EnumAdapters1(adapterIndex, &m_Gpu) != DXGI_ERROR_NOT_FOUND)
		{
			auto WideToString = [](const WCHAR* input, size_t size)
			{
				char* result = new char[size];
				wcstombs_s(0, result, size, input, size);
				return std::string(result);
			};

			DXGI_ADAPTER_DESC1 props;
			m_Gpu->GetDesc1(&props);

			if (props.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				adapterIndex++;
				continue;
			}

			if (FAILED(D3D12CreateDevice(
				m_Gpu,
				featureLevel,
				__uuidof(ID3D12Device), nullptr
			)))
			{
				adapterIndex++;
				continue;
			}

			m_GpuInfo.VendorId = props.VendorId;
			m_GpuInfo.DeviceId = props.DeviceId;
			m_GpuInfo.Info = WideToString(props.Description, 128);
			//HZ_INFO(m_GpuInfo.Print())
			//HZ_INFO("--------------------------------------------------------------------------")
			break;
		}

		if (FAILED(D3D12CreateDevice(
			m_Gpu,
			featureLevel,
			__uuidof(ID3D12Device), (void**)&m_Device
		)))
		{
			//HZ_ERROR("Failed to create D3D12 device!")
			return;
		}
	}

	void D3D12Renderer::CreateCommandQueue()
	{
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.NodeMask = 0;

		if (FAILED(m_Device->CreateCommandQueue(
			&commandQueueDesc,
			__uuidof(ID3D12CommandQueue), (void**)&m_CommandQueue
		)))
		{
			//HZ_ERROR("Failed to create D3D12 command queue!")
			return;
		}
	}

	void D3D12Renderer::CreateSwapchain()
	{
		D3D12_DESCRIPTOR_HEAP_DESC swapchainHeapDesc = {};
		swapchainHeapDesc.NumDescriptors = static_cast<UINT>(Renderer::GetBufferCount());
		swapchainHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		swapchainHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		if (FAILED(m_Device->CreateDescriptorHeap(
			&swapchainHeapDesc,
			__uuidof(ID3D12DescriptorHeap), (void**)&m_SwapchainHeap
		)))
		{
			////HZ_ERROR("Failed to create swapchain descriptor heap!")
			std::cout << "Failed to create swapchain descriptor heap!" << std::endl;
			return;
		}

		// TODO: Get the HANDLE directly from Window class.

		RECT windowClientRect;
		if (!GetClientRect(m_Handler, &windowClientRect))
		{
			return;
		}

		DXGI_SWAP_CHAIN_DESC swapchainDesc = {};
		swapchainDesc.BufferDesc.Width = windowClientRect.right - windowClientRect.left;
		swapchainDesc.BufferDesc.Height = windowClientRect.bottom - windowClientRect.top;
		swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapchainDesc.SampleDesc.Count = 1;
		swapchainDesc.SampleDesc.Quality = 0;
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.BufferCount = static_cast<UINT>(Renderer::GetBufferCount());
		swapchainDesc.OutputWindow = m_Handler;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.Windowed = TRUE;
		swapchainDesc.Flags = 0;

		Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
		if (FAILED(m_Factory->CreateSwapChain(
			m_CommandQueue,
			&swapchainDesc,
			&swapchain
		)))
		{
			//HZ_ERROR("Failed to create swapchain!")
			std::cout << "Failed to create swapchain!" << std::endl;
			return;
		}

		if (FAILED(swapchain->QueryInterface(
			__uuidof(IDXGISwapChain3), (void**)&m_Swapchain
		)))
		{
			//HZ_ERROR("Failed to query swapchain interface!")
			std::cout << "Failed to query swapchain interface!" << std::endl;
			return;
		}

		D3D12Renderer::Resize(
			static_cast<int>(swapchainDesc.BufferDesc.Width),
			static_cast<int>(swapchainDesc.BufferDesc.Height)
		);
	}

}