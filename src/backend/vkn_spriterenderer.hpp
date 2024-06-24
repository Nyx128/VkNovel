#pragma once
#include "vkn_context.hpp"
#include "vkn_cpubuffer.hpp"
#include "vkn_pipeline.hpp"

namespace vkn {
	class SpriteRenderer {
	public:
		SpriteRenderer(const SpriteRenderer&) = delete;
		SpriteRenderer& operator=(const SpriteRenderer&) = delete;

		SpriteRenderer(vkn::Context& _context);
		~SpriteRenderer();

		void render(const std::vector<float>& vertices, const std::vector<uint32_t>& indices);
	private:
		vkn::Context& context;

		const int MAX_FRAMES_IN_FLIGHT = 3;

		vk::CommandPool commandPool;
		std::vector<vk::CommandBuffer> commandBuffers;

		std::vector<vk::Framebuffer> swapchainFramebuffers;

		std::vector<vk::Semaphore> imageAvailableSemaphores;
		std::vector<vk::Semaphore> renderFinishedSemaphores;
		std::vector<vk::Fence> inFlightFences;

		uint32_t frame_index = 0;

		std::unique_ptr<vkn::Pipeline> pipeline;
		vk::DescriptorSetLayout setLayout;

		//functions
		void initSwapchainFramebuffers();
		void initPipeline();
		void initCommandPool();
		void initSyncObjects();

		std::unique_ptr<vkn::CPUBuffer> vertex_buffer;
		std::unique_ptr<vkn::CPUBuffer> index_buffer;
	};
}
