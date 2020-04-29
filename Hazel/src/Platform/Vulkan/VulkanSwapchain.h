#pragma once

#include <vulkan/vulkan.hpp>

namespace Hazel {

	
	typedef struct SwapchainBuffer
	{
		VkImage image;
		VkImageView view;
	};

	
	class HAZEL_API VulkanSwapchain
	{
	public:
		virtual void InitSurface(void* platformHandle, void* platformWindow);

		virtual void Connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);

		virtual void Create(uint32_t* width, uint32_t* height, bool vsync = false);

		virtual VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);

		virtual VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);

		virtual void Cleanup();

		uint32_t GetBufferCount() const { return imageCount; }
		const SwapchainBuffer& GetBuffer(uint32_t index) const { return buffers[index]; }

		VkFormat GetFormat() const { return colorFormat; }

		// TODO: REMOVE
		VkSwapchainKHR Get() { return swapchain; }

	private:
		VkInstance instance;
		VkDevice device;
		VkPhysicalDevice physicalDevice;
		VkSurfaceKHR surface;

		VkFormat colorFormat;
		VkColorSpaceKHR colorSpace;
		// Handle to the current swap chain, required for recreation
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;	
		uint32_t imageCount;
		std::vector<VkImage> images;
		std::vector<SwapchainBuffer> buffers;
		// Queue family index of the detected graphics and presenting device queue
		uint32_t queueNodeIndex = UINT32_MAX;

		uint32_t m_Width, m_Height;
	};
	
}
