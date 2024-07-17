#pragma once
#include "vkn_context.hpp"
#include "vkn_cpubuffer.hpp"
#include "vkn_pipeline.hpp"

#include "glm/glm.hpp"

namespace vkn {
	class SpriteRenderer {
	public:
		SpriteRenderer(const SpriteRenderer&) = delete;
		SpriteRenderer& operator=(const SpriteRenderer&) = delete;

		SpriteRenderer(vkn::Context& _context);
		~SpriteRenderer();

		struct SpriteData {
			glm::mat4 transform;
		};

		void begin();
		void draw(std::vector<SpriteData>& sprites);
		void end();
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
		uint32_t imageIndex = 0;

		std::unique_ptr<vkn::Pipeline> pipeline;
		vk::DescriptorSetLayout setLayout;

		std::vector<float> quad_vertices = {
			1.0f ,  1.0f, 0.0f,  // top right
			1.0f , -1.0f, 0.0f,  // bottom right
			-1.0f, -1.0f, 0.0f,  // bottom left
			-1.0f,  1.0f, 0.0f   // top left 
		};

		std::vector<uint32_t> indices = {
			0, 1, 3,   // first triangle
			1, 2, 3    // second triangle
		};

		const uint32_t batch_size = 32; 

		vkn::CPUBuffer quad_vbo{ context, sizeof(quad_vertices) * sizeof(float), vk::BufferUsageFlagBits::eStorageBuffer };
		vkn::CPUBuffer quad_ibo{ context, sizeof(quad_vertices) * sizeof(float), vk::BufferUsageFlagBits::eIndexBuffer };
		std::vector<std::unique_ptr<vkn::CPUBuffer>> idata_buffers;

		//functions
		void initSwapchainFramebuffers();
		void initPipeline();
		void initCommandPool();
		void initSyncObjects();
		
		glm::mat4 proj = glm::mat4(1.0f);

		vk::DeviceSize cvbSize = 0;
		vk::DeviceSize cibSize = 0;
	};
}
