#pragma once

#include <vulkan\vulkan.hpp>
#include <iostream>
#include <fstream>

using namespace vk;

namespace Prometheus {

	class Vulkan_Utils {
	public:
		static PFN_vkCreateDebugReportCallbackEXT Create_Debug_Report_Callback;
		static PFN_vkDestroyDebugReportCallbackEXT Destroy_Debug_Report_Callback;
		static DebugReportCallbackEXT m_Error_Callback;

		static void Vulkan_Error(Result err);
		static void Setup_Debug(Instance* instance);
		static void Free_Debug_Callback(Instance* instance);
	};

	class Utils {
	public:
		static std::vector<char> ImportShader(const std::string& fileName);
		static const std::vector<const char*> m_Validation_Layers = {
			"VK_LAYER_LUNARG_standard_validation"
		};
	};

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) {
		std::cerr << "validation layer: " << msg << std::endl;
		return VK_FALSE;
	}
}