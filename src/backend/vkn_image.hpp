#pragma once
#include "vulkan/vulkan.hpp"
#include "vkn_context.hpp"

namespace vkn {
	class Image {
	public:
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
	private:
		vk::Image handle;
		VmaAllocation memory;
		vk::ImageLayout currentLayout;
	};
}
