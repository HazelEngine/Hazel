#pragma once

#include <vulkan/vulkan.hpp>

namespace Hazel {

	class HAZEL_API VulkanMemoryAllocator
	{
	public:
		void Init(VkPhysicalDevice physicalDevice, VkDevice device);

		void Allocate(VkMemoryRequirements reqs, VkDeviceMemory* memory, VkMemoryPropertyFlags flags = 0);
		void Free(VkDeviceMemory memory);

	private:
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	private:
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;
	};

}