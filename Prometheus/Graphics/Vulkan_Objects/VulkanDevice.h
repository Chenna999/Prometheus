#pragma once

#include <vulkan\vulkan.hpp>
#include <vector>

#include "../../Utils/Utils.h"
#include "../../Utils/VulkanTools.h"
#include "VulkanBuffer.h"

using namespace vk;

namespace Prometheus {namespace Graphics {

	class VulkanDevice {
	public:
		struct QueueIndecieSet {
			uint32_t Graphics = UINT32_MAX, Present = UINT32_MAX, Compute = UINT32_MAX, Transfer = UINT32_MAX;
		};

		struct QueueSet {
			Queue Graphics, Present, Compute, Transfer;
		};
	private:
		Device m_Device;

		PhysicalDevice m_PhysicalDevice;
		PhysicalDeviceProperties m_DeviceProperties;
		PhysicalDeviceFeatures m_DeviceFeatures;
		PhysicalDeviceMemoryProperties m_MemoryProperties;
		std::vector<QueueFamilyProperties> m_QueueFamilyProperties;
		
		CommandPool m_CommandPool;
		SurfaceKHR m_Surface;

		QueueIndecieSet m_QueueIndecies;
		QueueSet m_Queues;
		float m_DefaultPriorities = { 1.0 };

		uint32_t GetQueueFamilyIndex(QueueFlags queueFlags);

	public:
		VulkanDevice(PhysicalDevice device, SurfaceKHR surface);
		~VulkanDevice();
		uint32_t FindMemoryType(uint32_t typeFilter, MemoryPropertyFlags props);
		void CreateLogicalDevice(PhysicalDeviceFeatures enabledFeatures, bool useSwapchain = true, QueueFlags requestedQueues = QueueFlagBits::eGraphics | QueueFlagBits::eCompute);
		void CreateBuffer(BufferUsageFlags usageFlags, MemoryPropertyFlags memoryFlags, VulkanBuffer* buffer, DeviceSize size, void* data = nullptr);
		void CopyBuffer(VulkanBuffer* src, VulkanBuffer* dst, Queue queue, BufferCopy* copyRegion = nullptr);
		CommandPool CreateCommandPool(uint32_t queueFamily, CommandPoolCreateFlags createFlags = CommandPoolCreateFlagBits::eResetCommandBuffer);
		CommandBuffer CreateCommandBuffer(CommandBufferLevel level, bool begin = true);
		void FlushCommandBuffer(CommandBuffer commandBuffer, Queue queue, bool free = true);
		QueueSet GetQueueSet();
		QueueIndecieSet GetQueueIndecies();
		PhysicalDevice GetPhysicalDevice();
		SurfaceKHR GetSurface();
		Device GetDevice();
	};

}}