#include "vkn_buffer.hpp"

namespace vkn {
	Buffer::Buffer(vkn::Context& _context, vk::DeviceSize _bufferSize, vk::BufferUsageFlags _usageFlags, VmaMemoryUsage _memUsage):
	context(_context), bufferSize(_bufferSize), usageFlags(_usageFlags), memUsage(_memUsage){
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.setUsage(usageFlags);
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;
		bufferInfo.size = bufferSize;

		allocInfo.usage = memUsage;

		VmaAllocator& allocator = context.getAllocator();
		auto result = vmaCreateBuffer(allocator, &static_cast<VkBufferCreateInfo>(bufferInfo), &allocInfo, &buf, &alloc, nullptr);
		if (result != VK_SUCCESS) { throw std::runtime_error("Failed to create buffer"); }
		buffer = vk::Buffer(buf);

		result = vmaMapMemory(context.getAllocator(), alloc, &mappedMem);
		if (result != VK_SUCCESS) { throw std::runtime_error("Failed to map buffer memory"); }
	}

	Buffer::~Buffer(){
		vmaUnmapMemory(context.getAllocator(), alloc);
		vmaDestroyBuffer(context.getAllocator(), buf, alloc);
	}
}