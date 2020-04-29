#include "hzpch.h"
#include "VulkanDevice.h"

#include "VulkanUtils.h"

std::vector<const char*> s_DeviceExt =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_MAINTENANCE1_EXTENSION_NAME
};

namespace Hazel {

	void VulkanDevice::Init(VkInstance instance)
	{
		m_Instance = instance;

		PickPhysicalDevice();
		CreateLogicalDevice();
	}

	void VulkanDevice::Cleanup()
	{
		vkDestroyDevice(m_Device, nullptr);
	}

	void VulkanDevice::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		HZ_CORE_ASSERT(deviceCount > 0, "Failed to find GPUs with Vulkan support!")

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device)) {
				m_PhysicalDevice = device;
				LogDeviceInfo(device);
				break;
			}
		}

		HZ_CORE_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "Failed to find a suitable GPU.")
	}

	void VulkanDevice::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = indices.GraphicsFamily.value();
		queueInfo.queueCount = 1;

		float queuePriority = 1.0f;
		queueInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pQueueCreateInfos = &queueInfo;
		deviceInfo.queueCreateInfoCount = 1;
		deviceInfo.pEnabledFeatures = &deviceFeatures;
		deviceInfo.enabledExtensionCount = s_DeviceExt.size();
		deviceInfo.ppEnabledExtensionNames = s_DeviceExt.data();
		deviceInfo.enabledLayerCount = 0;

		VK_CHECK_RESULT(vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device));

		vkGetDeviceQueue(m_Device, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
	}

	bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		QueueFamilyIndices indices = FindQueueFamilies(device);
		if (indices.IsComplete())
		{
			m_QueueFamilyIndices = indices;
		}

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
			&& indices.IsComplete();
	}

	QueueFamilyIndices VulkanDevice::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int idx = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.GraphicsFamily = idx;
			}
			if (indices.IsComplete())
			{
				break;
			}
			idx++;
		}

		return indices;
	}

	void VulkanDevice::LogDeviceInfo(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties props = {};
		vkGetPhysicalDeviceProperties(device, &props);

		// Get vendor name from the id
		auto& vendor = GetVendorNameFromId(props.vendorID);
		
		// Get api version as a readable string
		char version[512];
		sprintf_s(
			version, "%d.%d.%d",
			VK_VERSION_MAJOR(props.apiVersion),
			VK_VERSION_MINOR(props.apiVersion),
			VK_VERSION_PATCH(props.apiVersion)
		);

		HZ_CORE_INFO("Vulkan Renderer:")
		HZ_CORE_INFO("  Vendor: {0}", vendor)
		HZ_CORE_INFO("  Renderer: {0}", props.deviceName)
		HZ_CORE_INFO("  Version: {0}", version)
	}

}