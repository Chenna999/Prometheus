#pragma once

#include <vector>
#include <vulkan\vulkan.hpp>

using namespace vk;

namespace Prometheus {namespace Tools {

	struct SwapchainSupportDetails {
		SurfaceCapabilitiesKHR capabilities;
		std::vector<SurfaceFormatKHR> formats;
		std::vector<PresentModeKHR> presentModes;
	};

	struct ImageSwizzle {
		ComponentSwizzle R = ComponentSwizzle::eIdentity,
			G = ComponentSwizzle::eIdentity,
			B = ComponentSwizzle::eIdentity,
			A = ComponentSwizzle::eIdentity;
	};

	SwapchainSupportDetails QuerySwapchain(PhysicalDevice device, SurfaceKHR surface);
	void CreateImageView(Device device, Image image, Format format, ImageAspectFlags aspectFlags, ImageView& view, ImageSwizzle swizzle = ImageSwizzle());
	VkBool32 CheckDeviceExtensionPresent(PhysicalDevice device, const char* extensionName);
}}