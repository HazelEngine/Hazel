#include "hzpch.h"
#include "VulkanPipeline.h"

#include <Hazel/Renderer/Renderer.h>

#include "VulkanContext.h"
#include "VulkanShader.h"
#include "VulkanUtils.h"

namespace Hazel {

	VulkanPipeline::VulkanPipeline(const PipelineSpecification& spec)
		: m_Specification(spec)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		VkRenderPass vk_RenderPass = vk_Context->GetRenderPass();

		auto& vk_Device = vk_Context->GetDevice();
		VulkanShader* vk_Shader = dynamic_cast<VulkanShader*>(spec.Shader.get());

		//////////////////////////////////////////////////////////////////////////////////
		// TODO: Get from layout

		VkVertexInputBindingDescription vertexInputBindingDesc = {};
		vertexInputBindingDesc.binding = 0;
		vertexInputBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputBindingDesc.stride = sizeof(float) * 11;

		VkVertexInputAttributeDescription vertexInputAttrDesc[5] = {};
		
		vertexInputAttrDesc[0].binding = vertexInputBindingDesc.binding;
		vertexInputAttrDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttrDesc[0].location = 0;
		vertexInputAttrDesc[0].offset = 0;

		vertexInputAttrDesc[1].binding = vertexInputBindingDesc.binding;
		vertexInputAttrDesc[1].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttrDesc[1].location = 1;
		vertexInputAttrDesc[1].offset = sizeof(float) * 3;

		vertexInputAttrDesc[2].binding = vertexInputBindingDesc.binding;
		vertexInputAttrDesc[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		vertexInputAttrDesc[2].location = 2;
		vertexInputAttrDesc[2].offset = sizeof(float) * 5;

		vertexInputAttrDesc[3].binding = vertexInputBindingDesc.binding;
		vertexInputAttrDesc[3].format = VK_FORMAT_R32_SFLOAT;
		vertexInputAttrDesc[3].location = 3;
		vertexInputAttrDesc[3].offset = sizeof(float) * 9;

		vertexInputAttrDesc[4].binding = vertexInputBindingDesc.binding;
		vertexInputAttrDesc[4].format = VK_FORMAT_R32_SFLOAT;
		vertexInputAttrDesc[4].location = 4;
		vertexInputAttrDesc[4].offset = sizeof(float) * 10;

		//////////////////////////////////////////////////////////////////////////////////

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDesc;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttrDesc;
		vertexInputInfo.vertexAttributeDescriptionCount = std::extent<decltype(vertexInputAttrDesc)>::value;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineViewportStateCreateInfo viewportInfo = {};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

		VkViewport viewport;
		viewport.x = 0;
		viewport.y = (float)vk_Context->GetHeight(); // TODO: Set as a pipeline param
		viewport.width = (float)vk_Context->GetWidth(); // TODO: Set as a pipeline param
		viewport.height = -(float)vk_Context->GetHeight(); // TODO: Set as a pipeline param;
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

		viewportInfo.pViewports = &viewport;
		viewportInfo.viewportCount = 1;

		VkRect2D scissor;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = (float)vk_Context->GetWidth(); // TODO: Set as a pipeline param;
		scissor.extent.height = (float)vk_Context->GetHeight(); // TODO: Set as a pipeline param;

		viewportInfo.pScissors = &scissor;
		viewportInfo.scissorCount = 1;

		VkPipelineColorBlendAttachmentState colorBlendState = {};
		colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
										 VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendState.blendEnable = VK_TRUE;
		colorBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendState.alphaBlendOp = VK_BLEND_OP_SUBTRACT;

		VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
		colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendInfo.pAttachments = &colorBlendState;
		colorBlendInfo.attachmentCount = 1;

		VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
		depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilInfo.depthTestEnable = VK_FALSE;
		depthStencilInfo.depthWriteEnable = VK_FALSE;
		depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilInfo.stencilTestEnable = VK_FALSE;
		depthStencilInfo.back.passOp = VK_STENCIL_OP_KEEP;
		depthStencilInfo.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilInfo.front = depthStencilInfo.back;

		VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
		rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationInfo.depthBiasEnable = VK_FALSE;
		rasterizationInfo.depthClampEnable = VK_FALSE;
		rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationInfo.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
		multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Sample count?

		VkPipelineShaderStageCreateInfo shaderInfo[2] = {};
        shaderInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderInfo[0].module = vk_Shader->GetVertexShaderModule();
        shaderInfo[0].pName = "main";
        shaderInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

        shaderInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderInfo[1].module = vk_Shader->GetFragmentShaderModule();
        shaderInfo[1].pName = "main";
        shaderInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = vk_Shader->GetPipelineLayout();
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pColorBlendState = &colorBlendInfo;
		pipelineInfo.pDepthStencilState = &depthStencilInfo;
		pipelineInfo.pRasterizationState = &rasterizationInfo;
		pipelineInfo.pMultisampleState = &multisampleInfo;
		pipelineInfo.pStages = shaderInfo;
		pipelineInfo.stageCount = 2;
		pipelineInfo.renderPass = vk_RenderPass;
		
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(
			vk_Device->GetLogicalDevice(),
			VK_NULL_HANDLE,
			1,
			&pipelineInfo,
			nullptr,
			&m_Pipeline
		));
	}

}