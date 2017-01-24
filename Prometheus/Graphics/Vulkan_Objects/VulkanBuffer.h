#pragma once

#include <vulkan\vulkan.hpp>

using namespace vk;

namespace Prometheus { namespace Graphics {

	struct VulkanBuffer {
		Buffer m_Buffer;
		Device m_DeviceHandle;
		DeviceMemory m_Memory;
		DescriptorBufferInfo m_Descriptor;
		DeviceSize m_Size = 0;
		DeviceSize m_Alignment = 0;
		void* m_Mapped = nullptr;

		BufferUsageFlags m_UsageFlags;
		MemoryPropertyFlags m_MemoryFlags;

		void Map(DeviceSize size = VK_WHOLE_SIZE, DeviceSize offset = 0);
		void Unmap();
		void Bind(DeviceSize offset = 0);
		void SetupDescriptor(DeviceSize size = VK_WHOLE_SIZE, DeviceSize offset = 0);
		void CopyTo(void* data, DeviceSize size);
		void Flush(DeviceSize size = VK_WHOLE_SIZE, DeviceSize offset = 0);
		void Invalidate(DeviceSize size = VK_WHOLE_SIZE, DeviceSize offset = 0);
		void Destroy();
	};

}}