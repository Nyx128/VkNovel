#include "vkn_texture.hpp"
#include "backend/vkn_cpubuffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace vkn {
	Texture::Texture(vkn::Context& _context, std::string img_path):context(_context){
		int x, y, n, ok;
		ok = stbi_info(img_path.c_str(), &x, &y, &n);

		if (ok != 1) {
			throw std::runtime_error("attempting to load image of unsupported format or file is corrupted/missing");
		}

		pixels = stbi_load(img_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (!pixels) {
			throw std::runtime_error("failed to load image pixels");
		}

		vk::DeviceSize imageBufSize = width * height * 4;
		vkn::CPUBuffer stagingBuffer(context, imageBufSize, vk::BufferUsageFlagBits::eTransferSrc);
		void* sbuf_ptr = stagingBuffer.getMemory();

		memcpy(sbuf_ptr, pixels, static_cast<size_t>(imageBufSize));

		vk::ImageCreateInfo imageInfo;
		imageInfo.extent = vk::Extent3D(width, height, 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = vk::Format::eR8G8B8A8Srgb;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		imageInfo.setSamples(vk::SampleCountFlagBits::e1);
		imageInfo.setSharingMode(vk::SharingMode::eExclusive);
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.imageType = vk::ImageType::e2D;

		img = std::make_unique<vkn::Image>(context, imageInfo);

		vk::ImageMemoryBarrier imgBarrier_1;
		imgBarrier_1.oldLayout = vk::ImageLayout::eUndefined;
		imgBarrier_1.newLayout = vk::ImageLayout::eTransferDstOptimal;
		imgBarrier_1.image = img->getHandle();
		imgBarrier_1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imgBarrier_1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imgBarrier_1.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imgBarrier_1.subresourceRange.baseMipLevel = 0;
		imgBarrier_1.subresourceRange.baseArrayLayer = 0;
		imgBarrier_1.subresourceRange.layerCount = 1;
		imgBarrier_1.subresourceRange.levelCount = 1;
		imgBarrier_1.srcAccessMask = vk::AccessFlagBits::eNone;
		imgBarrier_1.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		{
			vk::CommandBuffer commandBuffer;
			context.beginSingleTimeCommandBuffer(commandBuffer);
			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eTransfer,
				vk::DependencyFlagBits::eByRegion,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&imgBarrier_1);
			context.endSingleTimeCommandBuffer(commandBuffer);
		}

		vk::BufferImageCopy region;
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = vk::Offset3D(0, 0, 0);
		region.imageExtent = vk::Extent3D(width, height, 1);

		{
			vk::CommandBuffer commandBuffer;
			context.beginSingleTimeCommandBuffer(commandBuffer);
			commandBuffer.copyBufferToImage(stagingBuffer.getHandle(), img->getHandle(), vk::ImageLayout::eTransferDstOptimal, 1, &region);
			context.endSingleTimeCommandBuffer(commandBuffer);
		}

		vk::ImageMemoryBarrier imgBarrier_2;
		imgBarrier_2.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		imgBarrier_2.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imgBarrier_2.image = img->getHandle();
		imgBarrier_2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imgBarrier_2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imgBarrier_2.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imgBarrier_2.subresourceRange.baseMipLevel = 0;
		imgBarrier_2.subresourceRange.baseArrayLayer = 0;
		imgBarrier_2.subresourceRange.layerCount = 1;
		imgBarrier_2.subresourceRange.levelCount = 1;
		imgBarrier_2.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		imgBarrier_2.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		{
			vk::CommandBuffer commandBuffer;
			context.beginSingleTimeCommandBuffer(commandBuffer);
			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eFragmentShader,
				vk::DependencyFlagBits::eByRegion,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&imgBarrier_2);

			context.endSingleTimeCommandBuffer(commandBuffer);
		}

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = vk::Format::eR8G8B8A8Srgb;
		viewInfo.image = img->getHandle();

		imgView = context.getDevice().createImageView(viewInfo);

		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerInfo.unnormalizedCoordinates = vk::False;
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.compareEnable = vk::False;
		samplerInfo.compareOp = vk::CompareOp::eAlways;

		sampler = context.getDevice().createSampler(samplerInfo);

	}

	Texture::~Texture(){
		stbi_image_free(pixels);
		auto device = context.getDevice();
		device.waitIdle();
		device.destroySampler(sampler);
		device.destroyImageView(imgView);
	}
}