#pragma once
#include "vkn_context.hpp"
#include "vkn_pipeline.hpp"
#include "vkn_buffer.hpp"

namespace vkn {
	class Renderer {
	public:
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		Renderer(vkn::Context& _context, vkn::Pipeline& _pipeline);
		~Renderer();

		void render();
	private:
		vkn::Context& context;
		vkn::Pipeline& pipeline;
		
		vk::CommandPool commandPool;
		std::vector<vk::CommandBuffer> commandBuffers;

		std::vector<vk::Framebuffer> swapchainFramebuffers;

		std::vector<vk::Semaphore> imageAvailableSemaphores;
		std::vector<vk::Semaphore> renderFinishedSemaphores;
		std::vector<vk::Fence> inFlightFences;

		const int MAX_FRAMES_IN_FLIGHT = 3;
		uint32_t frame_index = 0;

		//functions
		void initSwapchainFramebuffers();
		void initCommandPool();
		void initSyncObjects();

		//prototyping
		std::array<float, 12> vertices = {
			0.5f,  0.5f, 0.0f,  // top right
			0.5f, -0.5f, 0.0f,  // bottom right
			-0.5f, -0.5f, 0.0f,  // bottom left
			-0.5f,  0.5f, 0.0f   // top left 
		};

		std::array<uint32_t, 6> indices = {
			0, 1, 3,   // first triangle
			1, 2, 3    // second triangle
		};

		void* pvbo = nullptr;
		vkn::Buffer vertex_buffer{ context, sizeof(float) * vertices.size(), vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_ONLY};

		void* pibo = nullptr;
		vkn::Buffer index_buffer{ context, sizeof(uint32_t) * indices.size(), vk::BufferUsageFlagBits::eIndexBuffer, VMA_MEMORY_USAGE_CPU_COPY };
	};
}