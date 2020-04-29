#pragma once

#include <vulkan/vulkan.hpp>

namespace Hazel {

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsFamily;

		bool IsComplete() { return GraphicsFamily.has_value(); }
	};

	class HAZEL_API VulkanDevice
	{
	public:
		void Init(VkInstance instance);
		void Cleanup();

		const VkPhysicalDevice& GetPhysicalDevice() const { return m_PhysicalDevice; }
		const VkDevice& GetLogicalDevice() const { return m_Device; }
		const VkQueue& GetGraphicsQueue() const { return m_GraphicsQueue; }

		QueueFamilyIndices GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }

	private:
		void PickPhysicalDevice();
		void CreateLogicalDevice();

		bool IsDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		void LogDeviceInfo(VkPhysicalDevice device);

	private:
		VkInstance m_Instance;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;

		QueueFamilyIndices m_QueueFamilyIndices;
	};

}