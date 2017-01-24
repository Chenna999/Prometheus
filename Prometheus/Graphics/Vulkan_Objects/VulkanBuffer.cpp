#include "VulkanBuffer.h"

namespace Prometheus { namespace Graphics {

	void VulkanBuffer::Map(DeviceSize size, DeviceSize offset) {
		m_Mapped = m_DeviceHandle.mapMemory(m_Memory, offset, size, (MemoryMapFlagBits)0);
	}

	void VulkanBuffer::Unmap() {
		if (m_Mapped) {
			m_DeviceHandle.unmapMemory(m_Memory);
			m_Mapped = nullptr;
		}
	}

	void VulkanBuffer::Bind(DeviceSize offset) {
		m_DeviceHandle.bindBufferMemory(m_Buffer, m_Memory, offset);
	}

	void VulkanBuffer::SetupDescriptor(DeviceSize size, DeviceSize offset) {
		m_Descriptor.offset = offset;
		m_Descriptor.buffer = m_Buffer;
		m_Descriptor.range = size;
	}

	void VulkanBuffer::CopyTo(void* data, DeviceSize size) {
		if (m_Mapped == nullptr) return;
		memcpy(m_Mapped, data, size);
	}

	void VulkanBuffer::Flush(DeviceSize size, DeviceSize offset) {
		MappedMemoryRange mappedRange = {};
		mappedRange.memory = m_Memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		std::vector<MappedMemoryRange> ranges;
		ranges.push_back(mappedRange);
		m_DeviceHandle.flushMappedMemoryRanges(ranges);
	}

	void VulkanBuffer::Invalidate(DeviceSize size, DeviceSize offset) {
		MappedMemoryRange mappedRange = {};
		mappedRange.memory = m_Memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		std::vector<MappedMemoryRange> ranges;
		ranges.push_back(mappedRange);
		m_DeviceHandle.invalidateMappedMemoryRanges(ranges);
	}

	void VulkanBuffer::Destroy() {
		if (m_Buffer) {
			m_DeviceHandle.destroyBuffer(m_Buffer);
		}
		if (m_Memory) {
			m_DeviceHandle.freeMemory(m_Memory);
		}
	}
}}