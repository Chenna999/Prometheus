#include "VulkanDevice.h"

namespace Prometheus { namespace Graphics {

	VulkanDevice::VulkanDevice(PhysicalDevice device) : m_PhysicalDevice(device) {

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
}}