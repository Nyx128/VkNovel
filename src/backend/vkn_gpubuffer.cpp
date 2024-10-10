#include "vkn_gpubuffer.hpp"

namespace vkn {
	GPUBuffer::GPUBuffer(vkn::Context& _context, vk::DeviceSize _bufferSize, vk::BufferUsageFlags _usageFlags)
	:context(_context), bufferSize(_bufferSize), usageFlags(_usageFlags){
		//create a staging buffer to transfer to cpu writable memory to gpu memory
		vk::BufferCreateInfo stagingBufferInfo;
		stagingBufferInfo.size = bufferSize;
		stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		stagingBufferInfo.sharingMode = vk::SharingMode::eExclusive;

		VmaAllocationCreateInfo stagingAllocInfo{};
		stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		if (vmaCreateBuffer(context.getAllocator(), &static_cast<VkBufferCreateInfo>(stagingBufferInfo),
			&stagingAllocInfo, &vstagingBuffer, &stagingAlloc, nullptr) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create staging buffer for gpu buffer");
		}
		stagingBuffer = vk::Buffer(vstagingBuffer);
		auto result = vmaMapMemory(context.getAllocator(), stagingAlloc, &stagingMappedMem);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to map staging buffer memory for gpu buffer");
		}

		vk::BufferCreateInfo gpuBufferInfo;
		gpuBufferInfo.size = bufferSize;
		gpuBufferInfo.usage = usageFlags | vk::BufferUsageFlagBits::eTransferDst;
		gpuBufferInfo.sharingMode = vk::SharingMode::eExclusive;

		VmaAllocationCreateInfo gpuAllocInfo{};
		gpuAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		if (vmaCreateBuffer(context.getAllocator(), &static_cast<VkBufferCreateInfo>(gpuBufferInfo),
			&gpuAllocInfo, &vgpuBuffer, &gpuAlloc, nullptr) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create gpu buffer");
		}

		gpuBuffer = vk::Buffer(vgpuBuffer);
	}

	GPUBuffer::~GPUBuffer(){
		auto allocator = context.getAllocator();
		vmaDestroyBuffer(allocator, vgpuBuffer, gpuAlloc);

		vmaUnmapMemory(allocator, stagingAlloc);
		vmaDestroyBuffer(allocator, vstagingBuffer, stagingAlloc);
	}

	//write to the staging buffer
	void GPUBuffer::write(void* data, size_t offset, size_t range){
		if ((offset + range) > bufferSize) {
			throw std::runtime_error("attempting to write to out of bounds memory in staging buffer");
		}

		memcpy((void*)((size_t*)stagingMappedMem + offset), data, range);
	}

	//flush contents of the staging buffer onto the gpu buffer
	void GPUBuffer::flush(){
		vk::CommandBuffer commandBuffer;
		context.beginSingleTimeCommandBuffer(commandBuffer);

		vk::BufferCopy copyRegion;
		copyRegion.dstOffset = 0;
		copyRegion.size = bufferSize;

		commandBuffer.copyBuffer(stagingBuffer, gpuBuffer, 1, &copyRegion);

		context.endSingleTimeCommandBuffer(commandBuffer);
	}
}