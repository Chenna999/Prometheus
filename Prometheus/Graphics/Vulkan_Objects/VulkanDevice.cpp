#include "VulkanDevice.h"

namespace Prometheus {
	namespace Graphics {

		VulkanDevice::VulkanDevice(PhysicalDevice device, SurfaceKHR surface) : m_PhysicalDevice(device), m_Surface(surface) {

			m_DeviceProperties = m_PhysicalDevice.getProperties();
			m_DeviceFeatures = m_PhysicalDevice.getFeatures();
			m_MemoryProperties = m_PhysicalDevice.getMemoryProperties();
			m_QueueFamilyProperties = m_PhysicalDevice.getQueueFamilyProperties();
		}

		VulkanDevice::~VulkanDevice() {
			if (m_CommandPool) {
				m_Device.destroyCommandPool(m_CommandPool);
			}
			if (m_Device) {
				m_Device.destroy();
			}
		}

		uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, MemoryPropertyFlags properties) {
			for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; i++) {
				if ((typeFilter & (1 << i)) && ((m_MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)) return i;
			}
			throw std::runtime_error("Error: Could not find appropriate memory type");
		}

		uint32_t VulkanDevice::GetQueueFamilyIndex(QueueFlags queueFlags) {
			if (queueFlags & QueueFlagBits::eCompute) {
				for (uint32_t i = 0; i < static_cast<uint32_t>(m_QueueFamilyProperties.size()); i++) {
					if ((m_QueueFamilyProperties[i].queueFlags & queueFlags) && (((VkQueueFlags)(m_QueueFamilyProperties[i].queueFlags) & (VkQueueFlags)QueueFlagBits::eGraphics) == 0)){
						return i;
						break;
					}
				}
			}
			if (queueFlags & QueueFlagBits::eTransfer) {
				for (uint32_t i = 0; i < static_cast<uint32_t>(m_QueueFamilyProperties.size()); i++) {
					if ((m_QueueFamilyProperties[i].queueFlags & queueFlags) && (((VkQueueFlags)(m_QueueFamilyProperties[i].queueFlags) & (VkQueueFlags)QueueFlagBits::eGraphics) == 0) && (((VkQueueFlags)(m_QueueFamilyProperties[i].queueFlags) & (VkQueueFlags)QueueFlagBits::eCompute) == 0)) {
						return i;
						break;
					}
				}
			}

			for (uint32_t i = 0; i < static_cast<uint32_t>(m_QueueFamilyProperties.size()); i++) {
				if ((m_QueueFamilyProperties[i].queueFlags & queueFlags)) {
					return i;
					break;
				}
			}

			throw std::runtime_error("Error: Could not find suitable Queue Family");
		}

		void VulkanDevice::CreateLogicalDevice(PhysicalDeviceFeatures enabledFeatures, bool useSwapchain, QueueFlags requestedQueues) {
			std::vector<DeviceQueueCreateInfo> queueInfos;

			if (requestedQueues & QueueFlagBits::eGraphics) {
				m_QueueIndecies.Graphics = GetQueueFamilyIndex(QueueFlagBits::eGraphics);
				DeviceQueueCreateInfo queueInfo;
				queueInfo.queueFamilyIndex = m_QueueIndecies.Graphics;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &m_DefaultPriorities;
				queueInfos.push_back(queueInfo);
			}

			if (requestedQueues & QueueFlagBits::eCompute) {
				m_QueueIndecies.Compute = GetQueueFamilyIndex(QueueFlagBits::eCompute);
				if (m_QueueIndecies.Graphics != m_QueueIndecies.Compute) {
					DeviceQueueCreateInfo queueInfo;
					queueInfo.queueFamilyIndex = m_QueueIndecies.Compute;
					queueInfo.queueCount = 1;
					queueInfo.pQueuePriorities = &m_DefaultPriorities;
					queueInfos.push_back(queueInfo);
				}
			}
			else {
				m_QueueIndecies.Compute = m_QueueIndecies.Graphics;
			}

			if (requestedQueues & QueueFlagBits::eTransfer) {
				m_QueueIndecies.Transfer = GetQueueFamilyIndex(QueueFlagBits::eTransfer);
				if (m_QueueIndecies.Graphics != m_QueueIndecies.Transfer && m_QueueIndecies.Compute != m_QueueIndecies.Transfer) {
					DeviceQueueCreateInfo queueInfo;
					queueInfo.queueFamilyIndex = m_QueueIndecies.Transfer;
					queueInfo.queueCount = 1;
					queueInfo.pQueuePriorities = &m_DefaultPriorities;
					queueInfos.push_back(queueInfo);
				}
			}
			else {
				if (m_QueueIndecies.Graphics != UINT32_MAX) {
					m_QueueIndecies.Transfer = m_QueueIndecies.Graphics;
				}
				else {
					m_QueueIndecies.Transfer = m_QueueIndecies.Compute;
				}
			}

			for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++) {
				if (m_QueueFamilyProperties[i].queueCount > 0 && m_PhysicalDevice.getSurfaceSupportKHR(i, m_Surface)) {
					m_QueueIndecies.Present = i;
					break;
				}
			}
			if (m_QueueIndecies.Present != m_QueueIndecies.Graphics || m_QueueIndecies.Present != m_QueueIndecies.Transfer || m_QueueIndecies.Present != m_QueueIndecies.Compute) {
				if (m_QueueIndecies.Present != UINT32_MAX) {
					DeviceQueueCreateInfo queueInfo;
					queueInfo.queueFamilyIndex = m_QueueIndecies.Present;
					queueInfo.queueCount = 1;
					queueInfo.pQueuePriorities = &m_DefaultPriorities;
					queueInfos.push_back(queueInfo);
				}
				else {
					throw std::runtime_error("Error: Present queue could not be found");
				}
			}

			std::vector<const char*> deviceExtensions;
			if (useSwapchain) {
				deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			}

			DeviceCreateInfo deviceInfo;
			deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
			deviceInfo.pQueueCreateInfos = queueInfos.data();
			deviceInfo.pEnabledFeatures = &enabledFeatures;

#ifdef DEBUG_MODE
			if (Tools::CheckDeviceExtensionPresent(m_PhysicalDevice, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
				deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
			}

			deviceInfo.enabledLayerCount = Utils::m_Validation_Layers.size();
			deviceInfo.ppEnabledLayerNames = Utils::m_Validation_Layers.data();
#endif //DEBUG_MODE

			if (deviceExtensions.size() > 0) {
				deviceInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
				deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
			}

			m_Device = m_PhysicalDevice.createDevice(deviceInfo);

			if (m_Device) {
				m_CommandPool = CreateCommandPool(m_QueueIndecies.Graphics);
				if (m_QueueIndecies.Compute != UINT32_MAX) m_Queues.Compute = m_Device.getQueue(m_QueueIndecies.Compute, 0);
				if (m_QueueIndecies.Graphics != UINT32_MAX) m_Queues.Graphics = m_Device.getQueue(m_QueueIndecies.Graphics, 0);
				if (m_QueueIndecies.Transfer != UINT32_MAX) m_Queues.Transfer = m_Device.getQueue(m_QueueIndecies.Transfer, 0);
				if (m_QueueIndecies.Present != UINT32_MAX) m_Queues.Present = m_Device.getQueue(m_QueueIndecies.Present, 0);
			}
		}

		void VulkanDevice::CreateBuffer(BufferUsageFlags usageFlags, MemoryPropertyFlags memoryFlags, VulkanBuffer* buffer, DeviceSize size, void* data) {
			buffer->m_DeviceHandle = m_Device;
			
			BufferCreateInfo bufferInfo;
			bufferInfo.usage = usageFlags;
			bufferInfo.size = size;

			buffer->m_Buffer = m_Device.createBuffer(bufferInfo);

			MemoryRequirements memReq;
			MemoryAllocateInfo memAlloc;
			memReq = m_Device.getBufferMemoryRequirements(buffer->m_Buffer);
			memAlloc.allocationSize = memReq.size;
			memAlloc.memoryTypeIndex = FindMemoryType(memReq.memoryTypeBits, memoryFlags);
			buffer->m_Memory = m_Device.allocateMemory(memAlloc);
			
			buffer->m_Alignment = memReq.alignment;
			buffer->m_Size = memAlloc.allocationSize;
			buffer->m_UsageFlags = usageFlags;
			buffer->m_MemoryFlags = memoryFlags;

			if (data != nullptr) {
				buffer->Map();
				memcpy(buffer->m_Mapped, data, size);
				buffer->Unmap();
			}

			buffer->SetupDescriptor();
			buffer->Bind();
		}

		void VulkanDevice::CopyBuffer(VulkanBuffer* src, VulkanBuffer* dst, Queue queue, BufferCopy* copyRegion) {
			if (src->m_Size > dst->m_Size || src->m_Buffer == (Buffer)VK_NULL_HANDLE || dst->m_Buffer == (Buffer)VK_NULL_HANDLE) {
				throw std::runtime_error("Error: Null or mis-sized buffer when copying");
			}

			CommandBuffer copyCmd = CreateCommandBuffer(CommandBufferLevel::ePrimary, true);
			BufferCopy bufferCopy{};
			if (copyRegion == nullptr) {
				bufferCopy.size = src->m_Size;
			}
			else {
				bufferCopy = *copyRegion;
			}

			copyCmd.copyBuffer(src->m_Buffer, dst->m_Buffer, 1, &bufferCopy);
			FlushCommandBuffer(copyCmd, queue);
		}

		CommandPool VulkanDevice::CreateCommandPool(uint32_t queueFamily, CommandPoolCreateFlags createFlags) {
			CommandPoolCreateInfo poolInfo;
			poolInfo.queueFamilyIndex = queueFamily;
			poolInfo.flags = createFlags;
			CommandPool pool;
			pool = m_Device.createCommandPool(poolInfo);
			return pool;
		}

		CommandBuffer VulkanDevice::CreateCommandBuffer(CommandBufferLevel level, bool begin) {
			CommandBufferAllocateInfo allocInfo;
			allocInfo.commandPool = m_CommandPool;
			allocInfo.level = level;
			allocInfo.commandBufferCount = 1;

			CommandBuffer cmdBuffer;
			cmdBuffer = m_Device.allocateCommandBuffers(allocInfo)[0];

			if (begin) {
				CommandBufferBeginInfo beginInfo;
				cmdBuffer.begin(beginInfo);
			}

			return cmdBuffer;
		}

		void VulkanDevice::FlushCommandBuffer(CommandBuffer commandBuffer, Queue queue, bool free) {
			if (commandBuffer = (CommandBuffer)VK_NULL_HANDLE) return;

			commandBuffer.end();
			SubmitInfo subInfo;
			subInfo.commandBufferCount = 1;
			subInfo.pCommandBuffers = &commandBuffer;

			FenceCreateInfo fenceInfo;
			Fence fence;
			fence = m_Device.createFence(fenceInfo);

			queue.submit(subInfo, fence);
			m_Device.waitForFences(fence, VK_TRUE, UINT32_MAX);

			m_Device.destroyFence(fence);
			if (free) {
				m_Device.freeCommandBuffers(m_CommandPool, commandBuffer);
			}
		}

		VulkanDevice::QueueSet VulkanDevice::GetQueueSet() {
			return m_Queues;
		}

		VulkanDevice::QueueIndecieSet VulkanDevice::GetQueueIndecies() {
			return m_QueueIndecies;
		}

		PhysicalDevice VulkanDevice::GetPhysicalDevice() {
			return m_PhysicalDevice;
		}

		SurfaceKHR VulkanDevice::GetSurface() {
			return m_Surface;
		}

		Device VulkanDevice::GetDevice() {
			return m_Device;
		}
}}