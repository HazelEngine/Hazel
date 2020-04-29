#include "hzpch.h"
#include "VulkanShader.h"

#include <Hazel/Renderer/Renderer.h>

#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanUtils.h"

#include <spirv_cross.hpp>
#include <map>

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel {

	VulkanShader::VulkanShader(const ShaderCreateInfo& info)
		: m_Name(info.Name)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& device = vk_Context->GetDevice();

		VkShaderModuleCreateInfo moduleInfo = {};
		moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

		moduleInfo.pCode = info.VertexShaderSource.data();
		moduleInfo.codeSize = info.VertexShaderSource.size() * sizeof(uint32_t);

		VK_CHECK_RESULT(vkCreateShaderModule(
			device->GetLogicalDevice(), 
			&moduleInfo, 
			nullptr, 
			&m_VertexShaderModule
		));

		moduleInfo.pCode = info.FragmentShaderSource.data();
		moduleInfo.codeSize = info.FragmentShaderSource.size() * sizeof(uint32_t);

		VK_CHECK_RESULT(vkCreateShaderModule(
			device->GetLogicalDevice(), 
			&moduleInfo, 
			nullptr, 
			&m_FragmentShaderModule
		));

		GetShaderResources(info.VertexShaderSource, VK_SHADER_STAGE_VERTEX_BIT);
		GetShaderResources(info.FragmentShaderSource, VK_SHADER_STAGE_FRAGMENT_BIT);

		GenerateShaderResources();

		CreateSamplers();
		CreateDescriptorSet();
		CreatePipelineLayout();
	}

	void VulkanShader::Bind() const
	{

	}

	void VulkanShader::Unbind() const
	{

	}

	void VulkanShader::SetInt(const std::string& name, int value)
	{

	}

	void VulkanShader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{

	}

	void VulkanShader::SetFloat(const std::string& name, float value)
	{

	}

	void VulkanShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{

	}

	void VulkanShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{

	}

	void VulkanShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{

	}

	void VulkanShader::SetMat3(const std::string& name, const glm::mat3& value)
	{

	}

	void VulkanShader::SetMat4(const std::string& name, const glm::mat4& value)
	{

	}

	void VulkanShader::SetUniformBuffer(const std::string& name, void* data, uint32_t size)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();
		
		if (m_UniformBuffers.find(name) != m_UniformBuffers.end())
		{
			void* mapped = m_UniformBuffers[name]->Map();
			memcpy(mapped, data, size);
			m_UniformBuffers[name]->Unmap(size);
			return;
		}

		HZ_CORE_ERROR("Shader uniform buffer {0} doesn't exists!", name)
	}

	void VulkanShader::BindTexture(const std::string& name, const Ref<Texture2D>& texture)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice(); // TODO: Remove
		VulkanTexture2D* vk_Texture = dynamic_cast<VulkanTexture2D*>(texture.get());

		for (ShaderResource resource : m_ShaderResources)
		{
			if (resource.Name == name)
			{
				// Bind texture
				m_Textures[name] = texture;

				// Update Descriptor Sets

				VkWriteDescriptorSet writeDescriptorSets = {};
				writeDescriptorSets.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets.dstSet = m_DescriptorSet;
				writeDescriptorSets.dstBinding = resource.Binding;
				writeDescriptorSets.descriptorType = resource.DescriptorType;
				writeDescriptorSets.descriptorCount = resource.DescriptorCount;
				writeDescriptorSets.pImageInfo = &vk_Texture->GetDescriptorInfo();

				vkUpdateDescriptorSets(vk_Device->GetLogicalDevice(), 1, &writeDescriptorSets, 0, nullptr);
				
				return;
			}
		}

		HZ_CORE_ERROR("Has no {0} texture in the shader {1}", name, m_Name)
	}

	Ref<Texture2D> VulkanShader::GetTexture(const std::string& name) const
	{
		return m_Textures.at(name);
	}

	void VulkanShader::CreateSamplers()
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		VkSampler sampler;
		VK_CHECK_RESULT(vkCreateSampler(
			vk_Device->GetLogicalDevice(),
			&samplerInfo,
			nullptr,
			&sampler
		));

		m_Samplers.push_back(sampler);
	}

	void VulkanShader::CreateDescriptorSet()
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();

		uint32_t index = 0;

		// Descriptor Set Layout Bindings

		std::vector<VkDescriptorSetLayoutBinding> layoutBindings(m_ShaderResources.size());
		for (ShaderResource resource : m_ShaderResources)
		{
			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = resource.Binding;
			layoutBinding.descriptorCount = resource.DescriptorCount;
			layoutBinding.descriptorType = resource.DescriptorType;
			layoutBinding.stageFlags = resource.StageFlags;

			if (resource.IsSampler)
			{
				// Set the immutable samplers here, these are default
				// samplers used through all the application
				layoutBinding.pImmutableSamplers = m_Samplers.data();
			}

			layoutBindings[index++] = layoutBinding;
		}

		// Descriptor Set Layout

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.pBindings = layoutBindings.data();
		descriptorSetLayoutInfo.bindingCount = layoutBindings.size();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(
			vk_Device->GetLogicalDevice(),
			&descriptorSetLayoutInfo,
			nullptr,
			&m_DescriptorSetLayout
		));

		// Descriptor Pool Sizes

		std::unordered_map<VkDescriptorType, uint32_t> descriptors;
		for (ShaderResource resource : m_ShaderResources)
		{
			uint32_t count = descriptors[resource.DescriptorType];
			descriptors[resource.DescriptorType] = count + resource.DescriptorCount;
		}

		// If has descriptors, create them
		if (descriptors.size() > 0)
		{
			index = 0;
			std::vector<VkDescriptorPoolSize> poolSizes(descriptors.size());
			for (auto type : descriptors)
			{
				VkDescriptorPoolSize poolSize = {};
				poolSize.type = type.first;
				poolSize.descriptorCount = type.second;

				poolSizes[index++] = poolSize;
			}

			// Descriptor Pool

			VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.maxSets = 1; // max allocated descriptor sets ?
			descriptorPoolInfo.pPoolSizes = poolSizes.data();
			descriptorPoolInfo.poolSizeCount = poolSizes.size();

			VkDescriptorPool descriptorPool;
			VK_CHECK_RESULT(vkCreateDescriptorPool(
				vk_Device->GetLogicalDevice(),
				&descriptorPoolInfo,
				nullptr,
				&descriptorPool
			));

			// Descriptor Set Allocate

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pSetLayouts = &m_DescriptorSetLayout;
			allocInfo.descriptorSetCount = 1;
			allocInfo.descriptorPool = descriptorPool;

			VK_CHECK_RESULT(vkAllocateDescriptorSets(
				vk_Device->GetLogicalDevice(),
				&allocInfo,
				&m_DescriptorSet
			));
		}

		// Update Descriptor Sets
		for (ShaderResource resource : m_ShaderResources)
		{
			// Sampler Descriptors doesn't need to be updated
			if (!resource.IsSampler)
			{
				VkWriteDescriptorSet writeDescriptorSets = {};
				writeDescriptorSets.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets.dstSet = m_DescriptorSet;
				writeDescriptorSets.dstBinding = resource.Binding;
				writeDescriptorSets.descriptorType = resource.DescriptorType;
				writeDescriptorSets.descriptorCount = resource.DescriptorCount;

				// The buffers will be created on shader creation, so update his descriptors here
				if (resource.IsBuffer)
				{
					auto vk_Buffer = dynamic_cast<VulkanUniformBuffer*>(m_UniformBuffers[resource.Name].get());
					writeDescriptorSets.pBufferInfo = &vk_Buffer->GetDescriptorInfo();
				}
				else if (resource.IsImage)
				{
					auto vk_Texture = dynamic_cast<VulkanTexture2D*>(m_Textures[resource.Name].get());
					writeDescriptorSets.pImageInfo = &vk_Texture->GetDescriptorInfo();
				}

				vkUpdateDescriptorSets(vk_Device->GetLogicalDevice(), 1, &writeDescriptorSets, 0, nullptr);
			}
		}
	}

	void VulkanShader::CreatePipelineLayout()
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();

		// TODO: Set descriptor layouts & push constants
		VkPipelineLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		layoutInfo.setLayoutCount = 1;

		VK_CHECK_RESULT(vkCreatePipelineLayout(
			vk_Device->GetLogicalDevice(),
			&layoutInfo,
			nullptr,
			&m_PipelineLayout
		));
	}

	// NOTE: Receiving a stage, but some descriptors can be in more than one shader stage
	void VulkanShader::GetShaderResources(const std::vector<uint32_t>& spirv, VkShaderStageFlags stage)
	{
		spirv_cross::Compiler compiler(spirv);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		for (const auto& image : resources.sampled_images)
		{
			unsigned binding = compiler.get_decoration(image.id, spv::DecorationBinding);
				
			// Has at least 1 descriptor
			uint32_t descriptorCount = 1;

			// Check if is an array and his size
			const auto& type = compiler.get_type(image.type_id);
			if (type.array.size() > 0)
			{
				descriptorCount = 0;
				
				for (uint32_t i = 0; i < type.array.size(); i++)
				{
					descriptorCount += type.array[i];
				}
			}

			m_ShaderResources.push_back({
				binding,
				image.name,
				stage,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				descriptorCount,
				0,
				false,
				true
			});
		}

		for (const auto& image : resources.separate_images)
		{
			unsigned binding = compiler.get_decoration(image.id, spv::DecorationBinding);
				
			// Has at least 1 descriptor
			uint32_t descriptorCount = 1;

			// Check if is an array and his size
			const auto& type = compiler.get_type(image.type_id);
			if (type.array.size() > 0)
			{
				descriptorCount = 0;
				
				for (uint32_t i = 0; i < type.array.size(); i++)
				{
					descriptorCount += type.array[i];
				}
			}

			m_ShaderResources.push_back({
				binding,
				image.name,
				stage,
				VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				descriptorCount,
				0,
				false,
				true
			});
		}

		for (const auto& sampler : resources.separate_samplers)
		{
			unsigned binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
				
			// Has at least 1 descriptor
			uint32_t descriptorCount = 1;

			// Check if is an array and his size
			const auto& type = compiler.get_type(sampler.type_id);
			if (type.array.size() > 0)
			{
				descriptorCount = 0;
				
				for (uint32_t i = 0; i < type.array.size(); i++)
				{
					descriptorCount += type.array[i];
				}
			}

			m_ShaderResources.push_back({
				binding,
				sampler.name,
				stage,
				VK_DESCRIPTOR_TYPE_SAMPLER,
				descriptorCount,
				0,
				false,
				false,
				true
			});
		}

		for (const auto& ubuffer : resources.uniform_buffers)
		{
			unsigned binding = compiler.get_decoration(ubuffer.id, spv::DecorationBinding);
			
			// Uses the name queried using the id, as it returns the variable name rather than block name
			auto name = compiler.get_name(ubuffer.id);

			const auto& baseType = compiler.get_type(ubuffer.base_type_id);
			size_t bytes = compiler.get_declared_struct_size(baseType);

			// Has at least 1 descriptor
			uint32_t descriptorCount = 1;

			// Check if is an array and his size
			const auto& type = compiler.get_type(ubuffer.type_id);
			if (type.array.size() > 0)
			{
				descriptorCount = 0;
				
				for (uint32_t i = 0; i < type.array.size(); i++)
				{
					descriptorCount += type.array[i];
				}
			}

			m_ShaderResources.push_back({
				binding,
				name,
				stage,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				descriptorCount,
				static_cast<uint32_t>(bytes),
				true,
				false
			});
		}
	}

	void VulkanShader::GenerateShaderResources()
	{
		for (ShaderResource resource : m_ShaderResources)
		{
			if (resource.IsBuffer)
			{
				m_UniformBuffers[resource.Name] = UniformBuffer::Create(resource.Size);
			}
			else if (resource.IsImage)
			{
				// TODO: Set the default magenta texture
				uint32_t content = 0xFFFF00FF;
				m_Textures[resource.Name] = Texture2D::Create(&content, 1, 1, 4);
			}
		}
	}
}