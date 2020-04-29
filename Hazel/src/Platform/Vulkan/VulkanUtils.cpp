#include "hzpch.h"
#include "VulkanUtils.h"

#include <vulkan/vulkan.h>

std::string GetErrorString(VkResult errorCode)
{
#define STR(r) case VK_ ##r: return #r
	switch (errorCode)
	{
		STR(NOT_READY);
		STR(TIMEOUT);
		STR(EVENT_SET);
		STR(EVENT_RESET);
		STR(INCOMPLETE);
		STR(ERROR_OUT_OF_HOST_MEMORY);
		STR(ERROR_OUT_OF_DEVICE_MEMORY);
		STR(ERROR_INITIALIZATION_FAILED);
		STR(ERROR_DEVICE_LOST);
		STR(ERROR_MEMORY_MAP_FAILED);
		STR(ERROR_LAYER_NOT_PRESENT);
		STR(ERROR_EXTENSION_NOT_PRESENT);
		STR(ERROR_FEATURE_NOT_PRESENT);
		STR(ERROR_INCOMPATIBLE_DRIVER);
		STR(ERROR_TOO_MANY_OBJECTS);
		STR(ERROR_FORMAT_NOT_SUPPORTED);
		STR(ERROR_SURFACE_LOST_KHR);
		STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
		STR(SUBOPTIMAL_KHR);
		STR(ERROR_OUT_OF_DATE_KHR);
		STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
		STR(ERROR_VALIDATION_FAILED_EXT);
		STR(ERROR_INVALID_SHADER_NV);
	default:
		return "UNKNOWN_ERROR";
	}
#undef STR
}

std::string GetVendorNameFromId(uint32_t vendorId)
{
	char cstr[512];
	
	switch (vendorId)
	{
	case 0x8086:
	case 0x8087:
		sprintf_s(cstr, "Intel [0x%04x]", vendorId);
		break;
	case 0x1002:
	case 0x1022:
		sprintf_s(cstr, "AMD [0x%04x]", vendorId);
		break;
	case 0x10DE:
		sprintf_s(cstr, "Nvidia [0x%04x]", vendorId);
		break;
	case 0x1EB5:
		sprintf_s(cstr, "ARM [0x%04x]", vendorId);
		break;
	case 0x5143:
		sprintf_s(cstr, "Qualcomm [0x%04x]", vendorId);
		break;
	case 0x1099:
	case 0x10C3:
	case 0x1249:
	case 0x4E8:
		sprintf_s(cstr, "Samsung [0x%04x]", vendorId);
		break;
	default:
		sprintf_s(cstr, "0x%04x", vendorId);
		break;
	}

	return std::string(cstr);
}