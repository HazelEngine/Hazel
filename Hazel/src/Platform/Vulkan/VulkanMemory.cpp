#include "hzpch.h"
#include "VulkanMemory.h"

#include "VulkanUtils.h"

namespace Hazel {

	void VulkanMemoryAllocator::Init(VkPhysicalDevice physicalDevice, VkDevice device)
	{
		m_PhysicalDevice = physicalDevice;
		m_Device = device;
	}

	void VulkanMemoryAllocator::Allocate(VkMemoryRequirements reqs, VkDeviceMemory* memory, VkMemoryPropertyFlags flags)
	{
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = reqs.size;
		allocInfo.memoryTypeIndex = FindMemoryType(reqs.memoryTypeBits, flags);

		VK_CHECK_RESULT(vkAllocateMemory(m_Device, &allocInfo, nullptr, memory));
	}

	void VulkanMemoryAllocator::Free(VkDeviceMemory memory)
	{
		vkFreeMemory(m_Device, memory, nullptr);
	}

	uint32_t VulkanMemoryAllocator::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		HZ_CORE_ASSERT(true, "Failed to find suitable memory type!")
	}

}