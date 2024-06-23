#include "vkn_pipeline.hpp"

#include <fstream>

namespace vkn {
	Pipeline::Pipeline(vkn::Context& _context, PipelineInfo _pInfo):context(_context), pInfo(_pInfo){
		loadShaders();
		setFixedFunctions();
	}

	Pipeline::~Pipeline(){
		auto device = context.getDevice();
		device.destroyPipeline(pipeline);
		device.destroyPipelineCache(pcache);
		device.destroyRenderPass(renderpass);
		device.destroyPipelineLayout(pipelineLayout);
		device.destroyShaderModule(vertexModule);
		device.destroyShaderModule(fragModule);
	}

	std::vector<char> Pipeline::loadFile(std::string filepath){
		std::ifstream file(filepath.c_str(), std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			std::string err = "Failed to open file at (";
			err += filepath;
			err += ")";
			throw std::runtime_error(err);
		}

		size_t size = file.tellg();
		std::vector<char> buffer(size);
		file.seekg(0);
		file.read(buffer.data(), size);
		file.close();
		return buffer;
	}

	void Pipeline::loadShaders(){
		auto vertex = loadFile(pInfo.vShaderPath);
		auto fragment = loadFile(pInfo.fShaderPath);

		vk::ShaderModuleCreateInfo moduleInfo;
		moduleInfo.codeSize = vertex.size();
		moduleInfo.pCode = reinterpret_cast<const uint32_t*>(vertex.data());

		auto device = context.getDevice();
		vertexModule = device.createShaderModule(moduleInfo);

		moduleInfo.codeSize = fragment.size();
		moduleInfo.pCode = reinterpret_cast<const uint32_t*>(fragment.data());

		fragModule = device.createShaderModule(moduleInfo);
	}

	void Pipeline::setFixedFunctions() {
		//shader stage
		vertexShaderStage.setModule(vertexModule);
		vertexShaderStage.setPName("main");
		vertexShaderStage.setStage(vk::ShaderStageFlagBits::eVertex);

		fragShaderStage.setModule(fragModule);
		fragShaderStage.setPName("main");
		fragShaderStage.setStage(vk::ShaderStageFlagBits::eFragment);

		//vertexInput
		vertexInputState = vk::PipelineVertexInputStateCreateInfo();

		//input assembly
		InputAssemblyState.setPrimitiveRestartEnable(vk::False);
		InputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList);

		//dynamic states
		std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		dynamicState.dynamicStateCount = dynamicStates.size();
		dynamicState.pDynamicStates = dynamicStates.data();

		//viewport state
		viewportState.scissorCount = 1;
		viewportState.viewportCount = 1;

		//rasterizer state
		rasterizationState.lineWidth = 1.0f;
		rasterizationState.depthClampEnable = vk::False;
		rasterizationState.rasterizerDiscardEnable = vk::False;
		rasterizationState.polygonMode = vk::PolygonMode::eFill;
		rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
		rasterizationState.frontFace = vk::FrontFace::eClockwise;
		rasterizationState.depthBiasEnable = vk::False;

		//multisample state
		multisampleState = vk::PipelineMultisampleStateCreateInfo();

		//color blend state
		colorBlendState.logicOpEnable = vk::False;
		colorBlendState.setAttachments(pInfo.colorBlendAttachments);

		//pipeline layout creation
		pipelineLayout = context.getDevice().createPipelineLayout(pInfo.layoutInfo);

		renderpass = context.getDevice().createRenderPass(pInfo.passInfo);

		std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertexShaderStage, fragShaderStage };
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.setStages(shaderStages);
		pipelineInfo.pVertexInputState = &vertexInputState;
		pipelineInfo.pInputAssemblyState = &InputAssemblyState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizationState;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pColorBlendState = &colorBlendState;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.renderPass = renderpass;
		pipelineInfo.subpass = 0;

		vk::PipelineCacheCreateInfo cacheInfo;
		pcache = context.getDevice().createPipelineCache(cacheInfo);

		pipeline = context.getDevice().createGraphicsPipeline(pcache, pipelineInfo).value;
	}
}