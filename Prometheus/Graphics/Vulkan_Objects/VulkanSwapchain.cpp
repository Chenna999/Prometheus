#include "VulkanSwapchain.h"
namespace Prometheus {	namespace Graphics {


void VulkanSwapchain::CreateSwapchain(int width, int height,const bool vsync) {
	SwapchainKHR oldSwapchain = m_Swapchain;
	m_RequestedDetails request = { width, height, vsync };

	SwapchainSupportDetails SwapchainDetails = QuerySwapchain(m_PhysicalDevice, m_Surface);
	SurfaceFormatKHR SwapchainFormat = ChooseSwapchainFormat(SwapchainDetails.formats);
	PresentModeKHR SwapchainPresentMode = ChoosePresentMode(SwapchainDetails.presentModes, vsync);
	ChooseSwapchainExtent(SwapchainDetails.capabilities, request);
	uint32_t ImageCount = SwapchainDetails.capabilities.minImageCount + 1;
	m_ColourFormat = SwapchainFormat.format;
	m_ColourSpace = SwapchainFormat.colorSpace;

	if (SwapchainDetails.capabilities.maxImageCount > 0 && ImageCount > SwapchainDetails.capabilities.maxImageCount) {
		ImageCount = SwapchainDetails.capabilities.maxImageCount;
	}

	SwapchainCreateInfoKHR createInfo;
	createInfo.surface = m_Surface;
	createInfo.minImageCount = ImageCount;
	createInfo.imageFormat = SwapchainFormat.format;
	createInfo.imageColorSpace = SwapchainFormat.colorSpace;
	createInfo.imageExtent = m_SwapchainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = ImageUsageFlagBits::eColorAttachment;
	if (m_GraphicsQueueIndex != m_PresentQueueIndex) {
		uint32_t queueIndicies[] = { (uint32_t)m_GraphicsQueueIndex, (uint32_t)m_PresentQueueIndex };
		createInfo.imageSharingMode = SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueIndicies;
	}
	else {
		createInfo.imageSharingMode = SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	if (SwapchainDetails.capabilities.supportedTransforms & SurfaceTransformFlagBitsKHR::eIdentity) {
		createInfo.preTransform = SurfaceTransformFlagBitsKHR::eIdentity;
	}else{
		createInfo.preTransform = SwapchainDetails.capabilities.currentTransform;
	}
	createInfo.compositeAlpha = CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = SwapchainPresentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = oldSwapchain;

	m_Swapchain = m_Device.createSwapchainKHR(createInfo);

	if ((VkSwapchainKHR)oldSwapchain != VK_NULL_HANDLE) {
		for (uint32_t i = 0; i < m_ImageCount; i++) {
			m_Device.destroyImageView(m_ImageViews[i]);
		}
		m_Device.destroySwapchainKHR(oldSwapchain);
	}


}

void VulkanSwapchain::PresentImage(Queue queue, uint32_t imageIndex, Semaphore waitSemaphore) {
	PresentInfoKHR info;
	info.swapchainCount = 1;
	info.pSwapchains = &m_Swapchain;
	info.pImageIndices = &imageIndex;
	if ((VkSemaphore)waitSemaphore != VK_NULL_HANDLE) {
		info.pWaitSemaphores = &waitSemaphore;
		info.waitSemaphoreCount = 1;
	}
	queue.presentKHR(&info);
}

void VulkanSwapchain::ChooseSwapchainExtent(const SurfaceCapabilitiesKHR capabilities, m_RequestedDetails request) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) m_SwapchainExtent = capabilities.currentExtent;
	Extent2D actualExtent = { (uint32_t)request.width, (uint32_t)request.height };
	actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
	m_SwapchainExtent = actualExtent;
}

PresentModeKHR VulkanSwapchain::ChoosePresentMode(const std::vector<PresentModeKHR> availableModes, bool vsync) {
	if (!vsync) {
		for (const auto& f_mode : availableModes) {
			if (f_mode == PresentModeKHR::eMailbox) return f_mode;
		}
		for (const auto& f_mode : availableModes) {
			if (f_mode == PresentModeKHR::eImmediate) return f_mode;
		}
	}
	return PresentModeKHR::eFifo;
}

SurfaceFormatKHR VulkanSwapchain::ChooseSwapchainFormat(const std::vector<SurfaceFormatKHR> availableFormats) {
	if (availableFormats.size() == 1 && availableFormats[0].format == Format::eUndefined) {
		return{ Format::eB8G8R8A8Unorm, ColorSpaceKHR::eSrgbNonlinear };
	}
	for (const auto& f_format : availableFormats) {
		if (f_format.format == Format::eB8G8R8A8Unorm && f_format.colorSpace == ColorSpaceKHR::eSrgbNonlinear) return f_format;
	}
	return availableFormats[0];
}

void VulkanSwapchain::CreateImageViews() {
	m_Images = m_Device.getSwapchainImagesKHR(m_Swapchain);
	m_ImageCount = m_Images.size();

	m_ImageViews.clear();
	m_ImageViews.resize(m_ImageCount);

	for (uint32_t i = 0; i < m_Images.size(); i++) {
		CreateImageView(m_Device, m_Images[i], m_ColourFormat, ImageAspectFlagBits::eColor, m_ImageViews[i]);
	}
}

VulkanSwapchain::VulkanSwapchain(Instance instance, Device device, PhysicalDevice pDevice, SurfaceKHR surface, QueueIndecieSet indecies) {
	m_Instance = instance;
	m_Device = device;
	m_PhysicalDevice = pDevice;
	m_Surface = surface;
	m_GraphicsQueueIndex = indecies.Graphics;
	m_PresentQueueIndex = indecies.Present;
	m_GraphicsQueue = m_Device.getQueue(m_GraphicsQueueIndex, 0);
	m_PresentQueue = m_Device.getQueue(m_PresentQueueIndex, 0);
}

void VulkanSwapchain::DestroySwapchain() {
	m_Device.waitIdle();
	for (uint32_t i = 0; i < m_ImageCount; i++) {
		m_Device.destroyImageView(m_ImageViews[i]);
	}
	m_Device.destroySwapchainKHR(m_Swapchain);
}

SwapchainKHR VulkanSwapchain::getSwapchain() {
	return m_Swapchain;
}

VulkanSwapchain::QueueIndecieSet VulkanSwapchain::getQueueIndecies() {
	QueueIndecieSet req;
	req.Graphics = m_GraphicsQueueIndex;
	req.Present = m_PresentQueueIndex;
	return req;
}

VulkanSwapchain::QueueSet VulkanSwapchain::getQueues() {
	QueueSet req;
	req.Graphics = m_GraphicsQueue;
	req.Present = m_PresentQueue;
	return req;
}

Extent2D VulkanSwapchain::getExtent() {
	return m_SwapchainExtent;
}

std::vector<Image> VulkanSwapchain::getImages() {
	return m_Images;
}

std::vector<ImageView> VulkanSwapchain::getImageViews() {
	return m_ImageViews;
}

Format VulkanSwapchain::getFormat() {
	return m_ColourFormat;
}

}}