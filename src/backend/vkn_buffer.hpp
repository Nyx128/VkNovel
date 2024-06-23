#pragma once
#include "vkn_context.hpp"

namespace vkn {
	class Buffer {
	public:

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;

		Buffer(vkn::Context& _context, vk::DeviceSize _bufferSize, vk::BufferUsageFlags _usageFlags, VmaMemoryUsage _memUsage);
		~Buffer();

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
		VmaMemoryUsage memUsage;

		void* mappedMem = nullptr;
	};
}
