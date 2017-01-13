#pragma once

#include "vulkan\vulkan.hpp"
#include "../../Utils/VulkanTools.h"

namespace Prometheus { namespace Graphics {

	using namespace Tools;
	using namespace vk;

	class VulkanSwapchain {
	public:
		struct m_RequestedDetails;
		struct QueueSet;
		struct QueueIndecieSet;

	private:
		Instance m_Instance;
		Device m_Device;
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
		uint32_t m_GraphicsQueueIndex = UINT32_MAX, m_PresentQueueIndex = UINT32_MAX;
		Queue m_GraphicsQueue, m_PresentQueue;

	public:
		VulkanSwapchain(Instance instance, Device device, PhysicalDevice pDevice, SurfaceKHR surface, QueueIndecieSet indecies);
		void CreateSwapchain(int width, int height, const bool vsync = false);
		void DestroySwapchain();
		void PresentImage(Queue queue, uint32_t imageIndex, Semaphore waitSemaphore = (VkSemaphore)VK_NULL_HANDLE);
		
		SwapchainKHR getSwapchain();
		QueueIndecieSet getQueueIndecies();
		QueueSet getQueues();
		Extent2D getExtent();
		std::vector<Image> getImages();
		std::vector<ImageView> getImageViews();
		Format getFormat();

	public:
		

		struct QueueIndecieSet {
			uint32_t Graphics = UINT32_MAX, Present = UINT32_MAX;
		};

		struct QueueSet {
			Queue Graphics, Present;
		};

		struct m_RequestedDetails {
			int width, height;
			bool vsync;
		};
	};

}}
