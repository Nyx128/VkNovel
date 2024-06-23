#include "backend/vkn_window.hpp"
#include "backend/vkn_context.hpp"
#include "backend/vkn_pipeline.hpp"
#include "backend/vkn_renderer.hpp"

#include "backend/vkn_buffer.hpp"

#include <iostream>
#include <stdexcept>
#include <chrono>

void run() {
	vkn::Window win(1280, 720, "VkNovel", true);

	vkn::ContextInfo contextInfo;
	contextInfo.appVersion = VK_MAKE_VERSION(1, 0, 0);
	contextInfo.app_name = std::string("sandbox");

	vkn::Context context(contextInfo, win);

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

	vk::DescriptorSetLayout setLayout;
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

	vkn::Pipeline pipeline(context, pInfo);


	vkn::Renderer renderer(context, pipeline);

	std::string title = "";
	int sample_rate = 30;
	int sample_index = 0;
	float sample_time = 0.0f;
	while (!win.isClosed()) {
		auto start = std::chrono::high_resolution_clock::now();

		renderer.render();
		win.pollEvents();

		auto end = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		if (sample_index > sample_rate) {
			float avg_dt = sample_time / sample_rate;
			title = std::string("VkNovel") + std::string(" FPS: ") + std::to_string((int)round(1000.0f / avg_dt));
			win.setTitle(title.c_str());
			sample_index = 0;
			sample_time = 0.0f;
		}
		sample_index++;
		sample_time += dt;
	}
	context.getDevice().destroyDescriptorSetLayout(setLayout);
}

int main() {
	try {
		run();
	}
	catch (vk::SystemError& sysErr) {
		std::cout << sysErr.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cout << "UNKNOWN ERROR" << std::endl;
		return EXIT_FAILURE;
	}
}