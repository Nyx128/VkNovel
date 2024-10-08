#pragma once
#include "vkn_context.hpp"
#include "vkn_cpubuffer.hpp"
#include "vkn_gpubuffer.hpp"
#include "vkn_pipeline.hpp"
#include "game/vkn_texture.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

namespace vkn {
	class SpriteRenderer {
	public:
		SpriteRenderer(const SpriteRenderer&) = delete;
		SpriteRenderer& operator=(const SpriteRenderer&) = delete;

		SpriteRenderer(vkn::Context& _context);
		~SpriteRenderer();

		struct SpriteData {
			alignas(16) glm::mat4 transform;
			alignas(16) glm::vec3 col;
			uint32_t tex_id;
		};

		void begin();
		void draw(const std::vector<SpriteData>& sprites, const std::vector<std::shared_ptr<vkn::Texture>>& tex);
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

		vk::DescriptorPool descPool;

		std::vector<float> quad_vertices = {
			1.0f ,  1.0f, 0.0f, 1.0f, 1.0f,  // top right
			1.0f , -1.0f, 0.0f, 0.0f, 1.0f, // bottom right
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
			-1.0f,  1.0f, 0.0f, 1.0f, 0.0f  // top left 
		};

		std::vector<uint32_t> indices = {
			0, 1, 3,   // first triangle
			1, 2, 3    // second triangle
		};

		const uint32_t batch_size = 32; 

		vkn::GPUBuffer quad_vbo{ context, quad_vertices.size() * sizeof(float), vk::BufferUsageFlagBits::eStorageBuffer };
		vkn::GPUBuffer quad_ibo{ context, indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer};
		std::vector<std::unique_ptr<vkn::CPUBuffer>> idata_buffers;

		//functions
		void initSwapchainFramebuffers();
		void initPipeline();
		void initCommandPool();
		void initSyncObjects();
		void initDescPool();
		
		glm::mat4 proj = glm::mat4(1.0f);

		vk::DeviceSize cvbSize = 0;
		vk::DeviceSize cibSize = 0;
	};
}
