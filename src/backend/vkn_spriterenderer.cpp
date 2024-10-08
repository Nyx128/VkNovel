#include "vkn_spriterenderer.hpp"
#include <array>

namespace vkn {
	SpriteRenderer::SpriteRenderer(vkn::Context& _context):context(_context){
		initPipeline();
		initSwapchainFramebuffers();
		initCommandPool();
		initSyncObjects();

		memcpy(quad_vbo.getMemory(), quad_vertices.data(), quad_vertices.size() * sizeof(float));
		memcpy(quad_ibo.getMemory(), indices.data(), indices.size() * sizeof(uint32_t));

		idata_buffers.resize(20);//default max is 20 batches worth which would be 32 * 20 = 640 sprites.
		for (auto& b : idata_buffers) {
			b = std::make_unique<vkn::CPUBuffer>(context, sizeof(SpriteData) * batch_size, vk::BufferUsageFlagBits::eStorageBuffer);
		}
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

	void SpriteRenderer::begin(){

		auto device = context.getDevice();
		device.waitForFences(1, &inFlightFences[frame_index], vk::True, UINT64_MAX);
		device.resetFences(1, &inFlightFences[frame_index]);

		imageIndex = device.acquireNextImageKHR(context.getSwapchain(), UINT64_MAX, imageAvailableSemaphores[frame_index], VK_NULL_HANDLE).value;

		commandBuffers[frame_index].reset();

		commandBuffers[frame_index].begin(vk::CommandBufferBeginInfo());

		
		std::array<vk::ClearValue, 2> clearValues = { vk::ClearValue(vk::ClearColorValue(0.03f, 0.03f, 0.03f, 1.0f)),
													 vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0)) };
		vk::RenderPassBeginInfo beginInfo;
		beginInfo.clearValueCount = clearValues.size();
		beginInfo.pClearValues = clearValues.data();
		beginInfo.renderPass = pipeline->getRenderPass();
		beginInfo.framebuffer = swapchainFramebuffers[imageIndex];
		beginInfo.renderArea.offset = vk::Offset2D(0, 0);
		beginInfo.renderArea.extent = context.getSwapchainExtent();

		commandBuffers[frame_index].beginRenderPass(beginInfo, vk::SubpassContents::eInline);
		commandBuffers[frame_index].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getHandle());

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

	}

	void SpriteRenderer::draw(std::vector<SpriteData>& sprites, const std::vector<std::shared_ptr<vkn::Texture>>& tex){
		auto device = context.getDevice();

		uint32_t numBatches = ceilf((float)sprites.size() / (float)batch_size);
		if (numBatches > idata_buffers.size()) {
			//reset all buffers and resize
			for (auto& b : idata_buffers) {
				b.reset();
			}

			idata_buffers.resize(numBatches);
			for (auto& b : idata_buffers) {
				b = std::make_unique<vkn::CPUBuffer>(context, sizeof(SpriteData) * batch_size, vk::BufferUsageFlagBits::eStorageBuffer);
			}
		}

		//populate the buffers 
		uint32_t remainder = sprites.size() % batch_size;
		for (int i = 0; i < numBatches; i++) {
			if (i != numBatches - 1 || remainder == 0) {
				memcpy(idata_buffers[i]->getMemory(), &sprites[i * batch_size], sizeof(SpriteData) * batch_size);
			}
			else {
				memcpy(idata_buffers[i]->getMemory(), &sprites[i * batch_size], sizeof(SpriteData) * remainder);
			}
		}

		vk::DescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = quad_vbo.getHandle();
		bufferInfo.offset = 0;
		bufferInfo.range = quad_vbo.getSize();

		vk::WriteDescriptorSet vbufferPush;
		vbufferPush.descriptorType = vk::DescriptorType::eStorageBuffer;
		vbufferPush.descriptorCount = 1;
		vbufferPush.dstBinding = 0;
		vbufferPush.pBufferInfo = &bufferInfo;

		commandBuffers[frame_index].bindIndexBuffer(quad_ibo.getHandle(), 0, vk::IndexType::eUint32);
		VkWriteDescriptorSet _vbufferBufferPush = vbufferPush;
		context.getDispatchLoader().vkCmdPushDescriptorSetKHR(VkCommandBuffer(commandBuffers[frame_index]), VK_PIPELINE_BIND_POINT_GRAPHICS, VkPipelineLayout(pipeline->getLayout()), 0, 1, &_vbufferBufferPush);
		
		/*std::array<vk::DescriptorImageInfo, 2> texInfos;
		vk::DescriptorImageInfo texInfo;
		texInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		texInfo.imageView = tex[0]->getView();
		texInfo.sampler = tex[0]->getSampler();
		texInfos[0] = texInfo;

		vk::DescriptorImageInfo texInfo_1;
		texInfo_1.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		texInfo_1.imageView = tex[1]->getView();
		texInfo_1.sampler = tex[1]->getSampler();
		texInfos[1] = texInfo_1;

		vk::WriteDescriptorSet texPush;
		texPush.descriptorCount = 2;
		texPush.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		texPush.dstBinding = 2;
		texPush.pImageInfo = texInfos.data();*/


		std::vector<vk::DescriptorImageInfo> texInfos(tex.size());
		vk::DescriptorImageInfo texInfo;
		texInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		for (int i = 0; i < texInfos.size(); i++) {
			texInfo.imageView = tex[i]->getView();
			texInfo.sampler = tex[i]->getSampler();
			texInfos[i] = texInfo;
		}

		vk::WriteDescriptorSet texPush;
		texPush.descriptorCount = texInfos.size();
		texPush.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		texPush.dstBinding = 2;
		texPush.pImageInfo = texInfos.data();

		VkWriteDescriptorSet _texPush = texPush;
		context.getDispatchLoader().vkCmdPushDescriptorSetKHR(VkCommandBuffer(commandBuffers[frame_index]), VK_PIPELINE_BIND_POINT_GRAPHICS, VkPipelineLayout(pipeline->getLayout()), 0, 1, &_texPush);

		for (int i = 0; i < numBatches; i++) {

			vk::DescriptorBufferInfo instanceBufferInfo;
			instanceBufferInfo.buffer = idata_buffers[i]->getHandle();
			instanceBufferInfo.offset = 0;
			instanceBufferInfo.range = idata_buffers[i]->getSize();

			vk::WriteDescriptorSet idataPush;
			idataPush.descriptorCount = 1;
			idataPush.descriptorType = vk::DescriptorType::eStorageBuffer;
			idataPush.dstBinding = 1;
			idataPush.pBufferInfo = &instanceBufferInfo;

			VkWriteDescriptorSet _idataPush = idataPush;

			context.getDispatchLoader().vkCmdPushDescriptorSetKHR(VkCommandBuffer(commandBuffers[frame_index]), VK_PIPELINE_BIND_POINT_GRAPHICS, VkPipelineLayout(pipeline->getLayout()), 0, 1, &_idataPush);
			if (i != numBatches - 1 || remainder == 0) {
				commandBuffers[frame_index].drawIndexed(indices.size(), batch_size, 0, 0, 0);
			}
			else {
				commandBuffers[frame_index].drawIndexed(indices.size(), remainder, 0, 0, 0);
			}
		}

	}

	void SpriteRenderer::end(){
		commandBuffers[frame_index].endRenderPass();

		commandBuffers[frame_index].end();

		vk::SubmitInfo submitInfo;

		std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eTopOfPipe };

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
			std::array<vk::ImageView, 2> attachments = {
				swapchainImageViews[i],
				context.getDepthView()
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

		vk::DescriptorSetLayoutBinding vBinding;
		vBinding.binding = 0;
		vBinding.descriptorCount = 1;
		vBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
		vBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

		vk::DescriptorSetLayoutBinding instBinding;
		instBinding.binding = 1;
		instBinding.descriptorCount = 1;
		instBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
		instBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

		vk::DescriptorSetLayoutBinding texBinding;
		texBinding.binding = 2;
		texBinding.descriptorCount = 2;
		texBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		texBinding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics;


		std::array<vk::DescriptorSetLayoutBinding, 3> layoutBindings = { vBinding, instBinding, texBinding };

		setLayoutInfo.bindingCount = layoutBindings.size();
		setLayoutInfo.pBindings = layoutBindings.data();

		setLayout = context.getDevice().createDescriptorSetLayout(setLayoutInfo);

		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &setLayout;

		vk::PipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = vk::CompareOp::eLess;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		depthStencil.stencilTestEnable = VK_FALSE;

		vk::AttachmentDescription colorAttachment;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
		colorAttachment.format = context.getSwapchainFormat().format;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;


		vk::AttachmentDescription depthAttachment{};
		depthAttachment.format = vk::Format::eD32Sfloat;
		depthAttachment.samples = vk::SampleCountFlagBits::e1;;
		depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
		depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::AttachmentReference colorAttachmentRef;
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::AttachmentReference depthAttachmentRef;
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		vk::RenderPassCreateInfo passInfo;
		passInfo.setAttachmentCount(attachments.size());
		passInfo.pAttachments = attachments.data();
		passInfo.setSubpassCount(1);
		passInfo.pSubpasses = &subpass;

		vkn::PipelineInfo pInfo{ std::string("shaders/flat.vert.spv"), std::string("shaders/flat.frag.spv") , colorBlendAttachments, layoutInfo, passInfo , depthStencil};

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

		vk::SemaphoreTypeCreateInfo sTypeInfo;
		sTypeInfo.semaphoreType = vk::SemaphoreType::eTimeline;


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

	void SpriteRenderer::initDescPool(){
		vk::DescriptorPoolSize poolSize{};
		poolSize.type = vk::DescriptorType::eCombinedImageSampler;
		poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		vk::DescriptorPoolCreateInfo poolInfo;
		poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;

		descPool = context.getDevice().createDescriptorPool(poolInfo);
	}
}