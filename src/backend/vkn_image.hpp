#pragma once
#include "vulkan/vulkan.hpp"
#include "vkn_context.hpp"

namespace vkn {
	class Image {
	public:
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;

		Image(vkn::Context& _context, vk::ImageCreateInfo imgCreateInfo);
		~Image();

		vk::Image& getHandle() { return handle; }

	private:
		vkn::Context& context;
		vk::ImageCreateInfo imgCI;

		vk::Image handle;
		VmaAllocation alloc;
	};
}
