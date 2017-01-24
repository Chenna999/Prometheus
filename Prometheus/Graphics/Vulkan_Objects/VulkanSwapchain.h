#pragma once

#include "vulkan\vulkan.hpp"
#include "../../Utils/VulkanTools.h"
#include "VulkanDevice.h"

namespace Prometheus { namespace Graphics {

	using namespace Tools;
	using namespace vk;

	class VulkanSwapchain {
	public:
		struct m_RequestedDetails;

	private:
		Instance m_Instance;
		VulkanDevice* m_Device;
		PhysicalDevice m_PhysicalDevice;
		SurfaceKHR m_Surface;
		
		SurfaceFormatKHR ChooseSwapchainFormat(std::vector<SurfaceFormatKHR> formats);
		PresentModeKHR ChoosePresentMode(std::vector<PresentModeKHR> presentModes, bool vsync);
		void ChooseSwapchainExtent(const SurfaceCapabilitiesKHR capabilities, m_RequestedDetails request);
		void CreateImageViews();
		Format m_ColourFormat;
		Extent2D m_SwapchainExtent;
		ColorSpaceKHR m_ColourSpace;
		SwapchainKHR m_Swapchain;
		uint32_t m_ImageCount;
		std::vector<Image> m_Images;
		std::vector<ImageView> m_ImageViews;
		VulkanDevice::QueueIndecieSet m_QueueIndecies;
		VulkanDevice::QueueSet m_Queues;

	public:
		VulkanSwapchain(Instance instance, VulkanDevice* device);
		void CreateSwapchain(int width, int height, const bool vsync = false);
		void DestroySwapchain();
		void PresentImage(Queue queue, uint32_t imageIndex, Semaphore waitSemaphore = (VkSemaphore)VK_NULL_HANDLE);
		
		SwapchainKHR getSwapchain();
		Extent2D getExtent();
		std::vector<Image> getImages();
		std::vector<ImageView> getImageViews();
		Format getFormat();

	public:
		
		struct m_RequestedDetails {
			int width, height;
			bool vsync;
		};
	};

}}
