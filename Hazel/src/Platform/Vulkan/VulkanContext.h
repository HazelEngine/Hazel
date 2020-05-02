#pragma once

#include <Hazel/Renderer/GraphicsContext.h>

#include "VulkanMemory.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderPass.h"

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace Hazel {

	class HAZEL_API VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* window);
		virtual ~VulkanContext();

		virtual void Init() override;
		virtual void Cleanup();

		virtual void Prepare() override;
		virtual void SwapBuffers() override;
		virtual void FlushCommandBuffer(VkCommandBuffer cmdBuffer);

		const Scope<VulkanDevice>& GetDevice() const { return m_Device; }
		const Scope<VulkanMemoryAllocator>& GetMemoryAllocator() const { return m_MemoryAllocator; }
		VkCommandBuffer GetCommandBuffer(bool begin);
		VkRenderPass GetRenderPass() const { return m_RenderPass; }

		std::vector<VkCommandBuffer> GetDrawCommandBuffers() const { return m_DrawCmdBuffers; }
		std::vector<VkFramebuffer> GetFramebuffers() const { return m_Framebuffers; }

		VkCommandBuffer GetCurrentDrawCmdBuffer() const { return m_DrawCmdBuffers[m_CurrentBackbuffer]; }
		VkFramebuffer GetCurrentFramebuffer() const { return m_Framebuffers[m_CurrentBackbuffer]; }

		uint32_t GetWidth() const { return m_SwapchainWidth; }
		uint32_t GetHeight() const { return m_SwapchainHeight; }

	private:
		void CreateInstance();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();

		bool CheckValidationLayerIsPresent();

	private:
		GLFWwindow* m_Window;

		VkInstance m_Instance;

		Scope<VulkanDevice> m_Device;
		Scope<VulkanSwapchain> m_Swapchain;
		Scope<VulkanMemoryAllocator> m_MemoryAllocator;

		VkRenderPass m_RenderPass;

		std::vector<VkFramebuffer> m_Framebuffers;

		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_DrawCmdBuffers;

		uint32_t m_SwapchainWidth, m_SwapchainHeight;
		uint32_t m_CurrentBackbuffer;

		VkSemaphore m_ImageAcquiredSemaphore;
		VkSemaphore m_RenderingCompleteSemaphore;

		std::vector<VkFence> m_FrameFences;
	};

}