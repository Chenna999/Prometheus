#include "VulkanTools.h"

namespace Prometheus { namespace Tools {

	SwapchainSupportDetails QuerySwapchain(PhysicalDevice device, SurfaceKHR surface) {
		SwapchainSupportDetails details;
		details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
		details.formats = device.getSurfaceFormatsKHR(surface);
		details.presentModes = device.getSurfacePresentModesKHR(surface);
		return details;
	}

	void CreateImageView(Device device, Image image, Format format, ImageAspectFlags aspectFlags, ImageView &view, ImageSwizzle swizzle) {
		ImageViewCreateInfo createInfo;
		createInfo.image = image;
		createInfo.viewType = ImageViewType::e2D;
		createInfo.format = format;
		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.components.a = swizzle.A;
		createInfo.components.g = swizzle.G;
		createInfo.components.b = swizzle.B;
		createInfo.components.r = swizzle.R;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		view = device.createImageView(createInfo);
	}

	VkBool32 CheckDeviceExtensionPresent(PhysicalDevice device, const char* name) {
		std::vector<ExtensionProperties> extensions;
		extensions = device.enumerateDeviceExtensionProperties();
		for (auto& ext : extensions) {
			if (!strcmp(name, ext.extensionName)) {
				return true;
			}
		}	
		return false;
	}
}}