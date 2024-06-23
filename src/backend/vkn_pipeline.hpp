#pragma once
#include "vkn_context.hpp"

namespace vkn {

	struct PipelineInfo {
		std::string vShaderPath;
		std::string fShaderPath;
		std::vector<vk::PipelineColorBlendAttachmentState>& colorBlendAttachments;
		vk::PipelineLayoutCreateInfo& layoutInfo;
		vk::RenderPassCreateInfo& passInfo;
	};

	class Pipeline {
	public:
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		Pipeline(vkn::Context& _context, PipelineInfo _pInfo);
		~Pipeline();

		//getters
		vk::RenderPass& getRenderPass() { return renderpass; }
		vk::Pipeline& getHandle() { return pipeline; }
		vk::PipelineLayout& getLayout() { return pipelineLayout; }
	private:
		vkn::Context& context;
		PipelineInfo pInfo;

		std::vector<char> loadFile(std::string filepath);

		//pipeline specific handles
		vk::ShaderModule vertexModule;
		vk::ShaderModule fragModule;
		vk::PipelineShaderStageCreateInfo vertexShaderStage;
		vk::PipelineShaderStageCreateInfo fragShaderStage;
		vk::PipelineVertexInputStateCreateInfo vertexInputState;
		vk::PipelineInputAssemblyStateCreateInfo InputAssemblyState;
		vk::PipelineTessellationStateCreateInfo tessellationState;
		vk::PipelineViewportStateCreateInfo viewportState;
		vk::PipelineRasterizationStateCreateInfo rasterizationState;
		vk::PipelineMultisampleStateCreateInfo multisampleState;
		vk::PipelineDepthStencilStateCreateInfo depthStencilState;
		vk::PipelineColorBlendStateCreateInfo colorBlendState;
		vk::PipelineDynamicStateCreateInfo dynamicState;

		vk::PipelineLayout pipelineLayout;
		vk::RenderPass renderpass;
		vk::GraphicsPipelineCreateInfo pipelineInfo;

		vk::PipelineCache pcache;
		vk::Pipeline pipeline;
		//
		void loadShaders();
		void setFixedFunctions();
	};
}
