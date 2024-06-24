#include "vkn_spriterenderer.hpp"

namespace vkn {
	SpriteRenderer::SpriteRenderer(vkn::Context& _context):context(_context){
		initPipeline();
		initSwapchainFramebuffers();
		initCommandPool();
		initSyncObjects();

		//allocate enough space for atleast 1024 sprites. reallocate to bigger size if required
		vertex_buffer = std::make_unique<vkn::CPUBuffer>(context, sizeof(float) * 1024 * 4, vk::BufferUsageFlagBits::eStorageBuffer);
		index_buffer = std::make_unique<vkn::CPUBuffer>(context, sizeof(uint32_t) * 1024 * 6 , vk::BufferUsageFlagBits::eIndexBuffer);
	}

	SpriteRenderer::~SpriteRenderer(){
		auto& device = context.getDevice();
		device.waitIdle();

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			device.destroySemaphore(imageAvailableSemaphores[i]);
			device.destroySemaphore(renderFinishedSemaphores[i]);
			device.destroyFence(inFlightFences[i]);
		}

		device.destroyCommandPool(commandPool);
		for (auto& fb : swapchainFramebuffers) {
			device.destroyFramebuffer(fb);
		}

		device.destroyDescriptorSetLayout(setLayout);
	}

	void SpriteRenderer::render(const std::vector<float>& vertices, const std::vector<uint32_t>& indices){
		auto device = context.getDevice();

		memcpy(vertex_buffer->getMemory(), vertices.data(), sizeof(float) * vertices.size());
		memcpy(index_buffer->getMemory(), indices.data(), sizeof(uint32_t) * indices.size());

		device.waitForFences(1, &inFlightFences[frame_index], vk::True, UINT64_MAX);
		device.resetFences(1, &inFlightFences[frame_index]);
		uint32_t imageIndex;
		imageIndex = device.acquireNextImageKHR(context.getSwapchain(), UINT64_MAX, imageAvailableSemaphores[frame_index], VK_NULL_HANDLE).value;

		commandBuffers[frame_index].reset();

		commandBuffers[frame_index].begin(vk::CommandBufferBeginInfo());

		vk::ClearValue clearVal(vk::ClearColorValue(0.03f, 0.03f, 0.03f, 1.0f));
		vk::RenderPassBeginInfo beginInfo;
		beginInfo.clearValueCount = 1;
		beginInfo.pClearValues = &clearVal;
		beginInfo.renderPass = pipeline->getRenderPass();
		beginInfo.framebuffer = swapchainFramebuffers[imageIndex];
		beginInfo.renderArea.offset = vk::Offset2D(0, 0);
		beginInfo.renderArea.extent = context.getSwapchainExtent();

		commandBuffers[frame_index].beginRenderPass(beginInfo, vk::SubpassContents::eInline);
		commandBuffers[frame_index].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getHandle());

		vk::DescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = vertex_buffer->getHandle();
		bufferInfo.offset = 0;
		bufferInfo.range = vertex_buffer->getSize();

		vk::WriteDescriptorSet pushWrite;
		pushWrite.descriptorCount = 1;
		pushWrite.dstBinding = 0;
		pushWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
		pushWrite.pBufferInfo = &bufferInfo;


		vk::Viewport viewport;
		viewport.width = context.getSwapchainExtent().width;
		viewport.height = context.getSwapchainExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.x = 0.0f;
		viewport.y = 0.0f;

		commandBuffers[frame_index].setViewport(0, 1, &viewport);

		vk::Rect2D scissor;
		scissor.extent = context.getSwapchainExtent();
		scissor.offset = vk::Offset2D(0, 0);

		commandBuffers[frame_index].setScissor(0, 1, &scissor);
		VkWriteDescriptorSet write = static_cast<VkWriteDescriptorSet>(pushWrite);

		commandBuffers[frame_index].bindIndexBuffer(index_buffer->getHandle(), 0, vk::IndexType::eUint32);
		context.getDispatchLoader().vkCmdPushDescriptorSetKHR(VkCommandBuffer(commandBuffers[frame_index]), VK_PIPELINE_BIND_POINT_GRAPHICS, VkPipelineLayout(pipeline->getLayout()), 0, 1, &write);

		commandBuffers[frame_index].drawIndexed(indices.size(), 1, 0, 0, 0);
		commandBuffers[frame_index].endRenderPass();

		commandBuffers[frame_index].end();

		vk::SubmitInfo submitInfo;

		std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAvailableSemaphores[frame_index];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderFinishedSemaphores[frame_index];
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[frame_index];
		submitInfo.pWaitDstStageMask = waitStages.data();

		context.getGraphicsQueue().submit(submitInfo, inFlightFences[frame_index]);

		vk::PresentInfoKHR presentInfo;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &context.getSwapchain();
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphores[frame_index];

		auto result = context.getPresentQueue().presentKHR(presentInfo);

		frame_index = (frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void SpriteRenderer::initSwapchainFramebuffers(){
		auto swapchainImageViews = context.getSwapchainImageViews();
		swapchainFramebuffers.resize(swapchainImageViews.size());

		for (int i = 0; i < swapchainFramebuffers.size(); i++) {
			std::array<vk::ImageView, 1> attachments = {
				swapchainImageViews[i]
			};

			vk::FramebufferCreateInfo fbInfo(vk::FramebufferCreateFlags(),
				pipeline->getRenderPass(),
				attachments,
				context.getSwapchainExtent().width,
				context.getSwapchainExtent().height,
				1);

			swapchainFramebuffers[i] = context.getDevice().createFramebuffer(fbInfo);
		}
	}

	void SpriteRenderer::initPipeline(){
		std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments(1);
		colorBlendAttachments[0].colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR;
		colorBlendAttachments[0].blendEnable = vk::True;
		colorBlendAttachments[0].srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
		colorBlendAttachments[0].dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
		colorBlendAttachments[0].colorBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachments[0].srcAlphaBlendFactor = vk::BlendFactor::eOne;
		colorBlendAttachments[0].dstAlphaBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachments[0].alphaBlendOp = vk::BlendOp::eAdd;

		vk::PipelineLayoutCreateInfo layoutInfo;
		vk::DescriptorSetLayoutCreateInfo setLayoutInfo;
		setLayoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR;

		vk::DescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = 0;
		layoutBinding.descriptorCount = 1;
		layoutBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
		layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

		setLayoutInfo.bindingCount = 1;
		setLayoutInfo.pBindings = &layoutBinding;

		setLayout = context.getDevice().createDescriptorSetLayout(setLayoutInfo);

		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &setLayout;


		vk::AttachmentDescription colorAttachment;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
		colorAttachment.format = context.getSwapchainFormat().format;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;

		vk::AttachmentReference colorAttachmentRef;
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		vk::SubpassDependency dependency;
		dependency.srcSubpass = vk::SubpassExternal;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.srcAccessMask = vk::AccessFlagBits::eNone;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		vk::RenderPassCreateInfo passInfo;
		passInfo.setAttachmentCount(1);
		passInfo.pAttachments = &colorAttachment;
		passInfo.setSubpassCount(1);
		passInfo.pSubpasses = &subpass;
		passInfo.dependencyCount = 1;
		passInfo.pDependencies = &dependency;

		vkn::PipelineInfo pInfo{ std::string("shaders/flat.vert.spv"), std::string("shaders/flat.frag.spv") , colorBlendAttachments, layoutInfo, passInfo };

		pipeline = std::make_unique<vkn::Pipeline>(context, pInfo);
	}

	void SpriteRenderer::initCommandPool(){
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.queueFamilyIndex = context.getGraphicsQueueFamilyIndex();
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		commandPool = context.getDevice().createCommandPool(poolInfo);

		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;

		context.getDevice().allocateCommandBuffers(&allocInfo, commandBuffers.data());
	}

	void SpriteRenderer::initSyncObjects(){
		vk::FenceCreateInfo fenceInfo;
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
		vk::SemaphoreCreateInfo semaphoreInfo;

		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

		auto device = context.getDevice();
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			inFlightFences[i] = device.createFence(fenceInfo);
			imageAvailableSemaphores[i] = device.createSemaphore(semaphoreInfo);
			renderFinishedSemaphores[i] = device.createSemaphore(semaphoreInfo);
		}
	}
}