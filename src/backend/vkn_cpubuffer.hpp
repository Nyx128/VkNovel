#pragma once
#include "vkn_context.hpp"

namespace vkn {
	class CPUBuffer {
	public:

		CPUBuffer(const CPUBuffer&) = delete;
		CPUBuffer& operator=(const CPUBuffer&) = delete;

		CPUBuffer(vkn::Context& _context, vk::DeviceSize _bufferSize, vk::BufferUsageFlags _usageFlags);
		~CPUBuffer();

		void* getMemory() { return mappedMem; }

		vk::Buffer& getHandle() { return buffer; }
		vk::DeviceSize getSize() { return bufferSize; }
	private:
		vk::Buffer buffer;
		VkBuffer buf;
		vkn::Context& context;
		VmaAllocation alloc;
		VmaAllocationCreateInfo allocInfo = {};

		vk::DeviceSize bufferSize;
		vk::BufferUsageFlags usageFlags;

		void* mappedMem = nullptr;
	};
}
