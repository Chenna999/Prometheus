#include "Utils.h"

namespace Prometheus {

	PFN_vkCreateDebugReportCallbackEXT Vulkan_Utils::Create_Debug_Report_Callback = VK_NULL_HANDLE;
	PFN_vkDestroyDebugReportCallbackEXT Vulkan_Utils::Destroy_Debug_Report_Callback = VK_NULL_HANDLE;
	DebugReportCallbackEXT Vulkan_Utils::m_Error_Callback;

	void Vulkan_Utils::Vulkan_Error(Result err) {
		switch (err) {
		case Result::eNotReady:
			throw std::runtime_error("Error: Vulkan not ready\n");
			break;
		case Result::eTimeout:
			throw std::runtime_error("Error: Vulkan Timed out\n");
			break;
		case Result::eEventSet:
			throw std::runtime_error("Error: Vulkan event set\n");
			break;
		case Result::eEventReset:
			throw std::runtime_error("Error: Vulkan event reset\n");
			break;
		case Result::eIncomplete:
			throw std::runtime_error("Error: Vulkan incomplete\n");
			break;
		case Result::eErrorOutOfHostMemory:
			throw std::runtime_error("Error: Vulkan out of host memory\n");
			break;
		case Result::eErrorOutOfDeviceMemory:
			throw std::runtime_error("Error: Vulkan out of device memory\n");
			break;
		case Result::eErrorInitializationFailed:
			throw std::runtime_error("Error: Vulkan Initialization failed\n");
			break;
		case Result::eErrorDeviceLost:
			throw std::runtime_error("Error: Vulkan device lost\n");
			break;
		case Result::eErrorMemoryMapFailed:
			throw std::runtime_error("Error: Vulkan memory map failed\n");
			break;
		case Result::eErrorLayerNotPresent:
			throw std::runtime_error("Error: Vulkan layer not present\n");
			break;
		case Result::eErrorExtensionNotPresent:
			throw std::runtime_error("Error: Vulkan extention not present\n");
			break;
		case Result::eErrorFeatureNotPresent:
			throw std::runtime_error("Error: Vulkan feature not present\n");
			break;
		case Result::eErrorIncompatibleDriver:
			throw std::runtime_error("Error: Vulkan incompatibale driver\n");
			break;
		case Result::eErrorTooManyObjects:
			throw std::runtime_error("Error: Vulkan too many objects\n");
			break;
		case Result::eErrorFormatNotSupported:
			throw std::runtime_error("Error: Vulkan format not supported\n");
			break;
		case Result::eErrorFragmentedPool:
			throw std::runtime_error("Error: Vulkan fragmented pool\n");
			break;
		case Result::eErrorSurfaceLostKHR:
			throw std::runtime_error("Error: Vulkan surface lost\n");
			break;
		case Result::eErrorNativeWindowInUseKHR:
			throw std::runtime_error("Error: Vulkan native window in use\n");
			break;
		case Result::eSuboptimalKHR:
			throw std::runtime_error("Error: Vulkan suboptimal\n");
			break;
		case Result::eErrorOutOfDateKHR:
			throw std::runtime_error("Error: Vulkan out of date\n");
			break;
		case Result::eErrorIncompatibleDisplayKHR:
			throw std::runtime_error("Error: Vulkan incompatible display\n");
			break;
		case Result::eErrorValidationFailedEXT:
			throw std::runtime_error("Error: Vulkan validation failed\n");
			break;
		case Result::eErrorInvalidShaderNV:
			throw std::runtime_error("Error: Vulkan invalid shader\n");
			break;
		default:
			break;
		}
	}

	void Vulkan_Utils::Setup_Debug(Instance* instance) {
		Vulkan_Utils::Create_Debug_Report_Callback = (PFN_vkCreateDebugReportCallbackEXT)instance->getProcAddr("vkCreateDebugReportCallbackEXT");
		Vulkan_Utils::Destroy_Debug_Report_Callback = (PFN_vkDestroyDebugReportCallbackEXT)instance->getProcAddr("vkDestroyDebugReportCallbackEXT");
		DebugReportCallbackCreateInfoEXT Debug_Info;
		Debug_Info.flags = DebugReportFlagBitsEXT::eError | DebugReportFlagBitsEXT::eWarning;
		Debug_Info.pfnCallback = debugCallback;
		auto err = instance->createDebugReportCallbackEXT(&Debug_Info, nullptr, &(m_Error_Callback));
		Vulkan_Error(err);
		if (!m_Error_Callback) std::cout << "Debug failed to initialise" << std::endl;
	}

	void Vulkan_Utils::Free_Debug_Callback(Instance* instance) {
		instance->destroyDebugReportCallbackEXT(Vulkan_Utils::m_Error_Callback);
	}

	std::vector<char> Utils::ImportShader(const std::string& fileName) {
		std::ifstream m_File( fileName, std::ios::ate | std::ios::binary);
		if (!m_File.is_open()) throw std::runtime_error("Error: Could not open file.\n");
		size_t fileSize = (size_t)m_File.tellg();
		std::vector<char> buffer(fileSize);
		m_File.seekg(0);
		m_File.read(buffer.data(), fileSize);
		m_File.close();
		return buffer;
	}
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* createInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
	return Prometheus::Vulkan_Utils::Create_Debug_Report_Callback(instance, createInfo, pAllocator, pCallback);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT Callback, const VkAllocationCallbacks* pAllocator) {
	Prometheus::Vulkan_Utils::Destroy_Debug_Report_Callback(instance, Callback, pAllocator);
}