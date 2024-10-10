#include "vkn_context.hpp"

namespace vkn {
	class GPUBuffer {
	public:
		GPUBuffer(const GPUBuffer&) = delete;
		GPUBuffer& operator=(const GPUBuffer&) = delete;

		GPUBuffer(vkn::Context& _context, vk::DeviceSize _bufferSize, vk::BufferUsageFlags _usageFlags);
		~GPUBuffer();

		void write(void* data, size_t offset, size_t range);
		void flush();

		vk::Buffer& getHandle() { return gpuBuffer; }
		vk::DeviceSize getSize() { return bufferSize; }
	private:
		vkn::Context& context;
		vk::DeviceSize bufferSize;
		vk::BufferUsageFlags usageFlags;

		VkBuffer vstagingBuffer;
		vk::Buffer stagingBuffer;
		VmaAllocation stagingAlloc;
		void* stagingMappedMem;

		VkBuffer vgpuBuffer;
		vk::Buffer gpuBuffer;
		VmaAllocation gpuAlloc;
	};
}