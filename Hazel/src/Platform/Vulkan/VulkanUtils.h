#pragma once

enum VkResult;
std::string GetErrorString(VkResult errorCode);
std::string GetVendorNameFromId(uint32_t vendorId);

#define VK_CHECK_RESULT(x)																					\
{																											\
	VkResult result = (x);																					\
	if (result != VK_SUCCESS)																				\
	{																										\
		HZ_CORE_ERROR(																						\
			"FATAL: VkResult is \"{0}\" in file {1} at line {2}",											\
			GetErrorString(result), __FILE__, __LINE__														\
		)																									\
		HZ_CORE_ASSERT(false, "")																			\
	}																										\
}