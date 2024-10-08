#pragma once

#include "backend/vkn_image.hpp"
#include <string>
#include <memory>

namespace vkn {
	class Texture {
	public:
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		Texture(vkn::Context& _context, std::string img_path);
		~Texture();

		vk::ImageView& getView() { return imgView; }
		vk::Sampler& getSampler() { return sampler; }

	private:
		vkn::Context& context;
		std::unique_ptr<vkn::Image> img;
		vk::ImageView imgView;
		uint8_t* pixels;
		int width, height, channels;
		vk::Sampler sampler;
	};
}