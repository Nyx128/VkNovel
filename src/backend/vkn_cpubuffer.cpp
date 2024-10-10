#include "vkn_cpubuffer.hpp"

namespace vkn {
	CPUBuffer::CPUBuffer(vkn::Context& _context, vk::DeviceSize _bufferSize, vk::BufferUsageFlags _usageFlags):
	context(_context), bufferSize(_bufferSize), usageFlags(_usageFlags){
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.setUsage(usageFlags);
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;
		bufferInfo.size = bufferSize;

		allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VmaAllocator& allocator = context.getAllocator();
		auto result = vmaCreateBuffer(allocator, &static_cast<VkBufferCreateInfo>(bufferInfo), &allocInfo, &vbuffer, &alloc, nullptr);
		if (result != VK_SUCCESS) { throw std::runtime_error("Failed to create buffer"); }
		buffer = vk::Buffer(vbuffer);

		result = vmaMapMemory(context.getAllocator(), alloc, &mappedMem);
		if (result != VK_SUCCESS) { throw std::runtime_error("Failed to map buffer memory"); }
	}

	CPUBuffer::~CPUBuffer(){
		vmaUnmapMemory(context.getAllocator(), alloc);
		vmaDestroyBuffer(context.getAllocator(), vbuffer, alloc);
	}
}