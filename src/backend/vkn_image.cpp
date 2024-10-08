#include "vkn_image.hpp"

namespace vkn {
	Image::Image(vkn::Context& _context, vk::ImageCreateInfo imgCreateInfo):context(_context), imgCI(imgCreateInfo){
		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VkImageCreateInfo c_info = imgCI;
		VkImage c_handle;

		auto result = vmaCreateImage(context.getAllocator(), &c_info, &allocInfo, &c_handle, &alloc, nullptr);
		if (result != VK_SUCCESS) { throw std::runtime_error("failed to create image"); }

		handle = vk::Image(c_handle);
	}

	Image::~Image(){
		vmaDestroyImage(context.getAllocator(), VkImage(handle), alloc);
	}
}