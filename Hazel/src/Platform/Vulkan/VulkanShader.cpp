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
		: m_Name(info.Name), m_MaterialCount(0)
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

		GetShaderResources(info.VertexShaderSource, ShaderDomain::Vertex);
		GetShaderResources(info.FragmentShaderSource, ShaderDomain::Pixel);

		GenerateShaderResources();

		CreateSamplers();
		CreateDescriptorSets();
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
		if (m_UniformBuffers.find(name) != m_UniformBuffers.end())
		{
			void* mapped = m_UniformBuffers[name]->Map();
			memcpy(mapped, data, size);
			m_UniformBuffers[name]->Unmap(size);
			return;
		}

		HZ_CORE_ERROR("Shader uniform buffer {0} doesn't exists!", name)
	}

	void VulkanShader::SetUniformBufferParam(const std::string& name, const std::string& param, void* data, uint32_t size)
	{
		ShaderUniformBufferDeclaration* ubuffer = nullptr;

		for (auto decl : m_VSUniformBuffers)
		{
			if (decl->GetName() == name)
				ubuffer = decl;
		}
		for (auto decl : m_PSUniformBuffers)
		{
			if (decl->GetName() == name)
				ubuffer = decl;
		}

		if (ubuffer)
		{
			auto paramDecl = ubuffer->FindUniform(param);
			if (paramDecl)
			{
				if (size == paramDecl->GetSize())
				{
					uint32_t offset = paramDecl->GetOffset();
					byte* mapped = (byte*)m_UniformBuffers[name]->Map();
					memcpy(mapped + offset, data, size);
					m_UniformBuffers[name]->Unmap(size);
					return;
				}
				HZ_CORE_ERROR("Size for param '{0}' has to be equal to {1}, currently is {2}.", param, paramDecl->GetSize(), size)
				return;
			}
			HZ_CORE_ERROR("Param '{0}' doesn't exists for uniform buffer '{1}'!", param, name)
			return;
		}
		HZ_CORE_ERROR("Shader uniform buffer {0} doesn't exists!", name)
	}

	void VulkanShader::BindTexture(const std::string& name, const Ref<Texture>& texture)
	{
		BindTexture(name, 0, texture);
	}

	void VulkanShader::BindTexture(const std::string& name, uint32_t index, const Ref<Texture>& texture)
	{
		VulkanTexture2D* vk_Texture = dynamic_cast<VulkanTexture2D*>(texture.get());

		for (auto resource : m_Resources)
		{
			if (resource->GetName() == name)
			{
				if (index > resource->GetCount())
				{
					HZ_CORE_ERROR("Has no slot {0} in the texture {1}.", index, name)
					return;
				}

				auto vk_Resource = dynamic_cast<VulkanShaderResourceDeclaration*>(resource);

				// Get the textures, update the texture in the index, and set textures back
				auto& textures = m_Textures[name];
				textures[index] = texture;
				m_Textures[name] = textures;

				// TODO: Remove '-1', this is necessary now, as we don't process the global descriptor as set = 0
				uint32_t set = vk_Resource->GetSet() - 1;

				// Update Descriptor Set with the texture data
				UpdateDescriptorSet((VulkanShaderResourceDeclaration*)resource, textures, m_TextureDescriptorSets[set]);
				
				return;
			}
		}

		HZ_CORE_ERROR("Has no {0} texture in the shader {1}", name, m_Name)
	}

	void VulkanShader::BindTextureToPool(const std::string& name, const Ref<Texture>& texture)
	{
		VulkanTexture2D* vk_Texture = dynamic_cast<VulkanTexture2D*>(texture.get());

		for (auto resource : m_Resources)
		{
			if (resource->GetName() == name)
			{
				// Try to find the descritor, if not created yet, create and save it
				auto set = GetTexturePoolDescriptorSet(texture);
				if (set == VK_NULL_HANDLE)
				{
					// Create and save the descriptor in the pool
					set = CreateDescriptorSetForTexture((VulkanShaderResourceDeclaration*)resource, texture);
				}

				// Update Descriptor Set with the texture data
				UpdateDescriptorSet((VulkanShaderResourceDeclaration*)resource, { texture }, set);
				
				return;
			}
		}

		HZ_CORE_ERROR("Has no {0} texture in the shader {1}", name, m_Name)
	}

	Ref<Texture> VulkanShader::GetTexture(const std::string& name) const
	{
		return GetTexture(name, 0);
	}

	Ref<Texture> VulkanShader::GetTexture(const std::string& name, uint32_t index) const
	{
		auto textures = m_Textures.at(name);
		HZ_CORE_ASSERT(
			index < textures.size(),
			"Has no texture data in the index " + std::to_string(index) + " from the texture " + name
		)
		return textures[index];
	}

	void VulkanShader::SetMaterialUniformBuffer(Buffer buffer, uint32_t materialIndex)
	{
		if (HasVSMaterialUniformBuffer() || HasPSMaterialUniformBuffer())
		{
			auto vk_Buffer = dynamic_cast<VulkanUniformBuffer*>(m_MaterialUniformBuffer.get());
			auto offset = materialIndex * m_MaterialUniformBufferAlignment;

			HZ_CORE_ASSERT(offset + buffer.Size <= vk_Buffer->GetSize(), "Material Uniform Buffer overflow!")

			byte* mapped = (byte*)m_MaterialUniformBuffer->Map();
			memcpy(mapped + offset, buffer.Data, buffer.Size);
			m_MaterialUniformBuffer->Unmap(buffer.Size);
			return;
		}

		HZ_CORE_ERROR("Shader has no material uniform buffer!")
	}

	uint32_t VulkanShader::GetTextureDescriptorSetIndex(const std::string& name) const
	{
		for (auto resource : m_Resources)
		{
			if (resource->GetName() == name)
			{
				auto vk_Resource = dynamic_cast<VulkanShaderResourceDeclaration*>(resource);
				return vk_Resource->GetSet() - 1; // TODO: Remove the '-1'
			}
		}

		HZ_CORE_ASSERT(true, "Descriptor set not found!")
		return -1;
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

	void VulkanShader::CreateDescriptorSets()
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();

		uint32_t index = 0;

		// Descriptor Set Layout Bindings

		// Uniform buffers count
		uint32_t ubufferCount = m_VSUniformBuffers.size() + m_PSUniformBuffers.size();
		
		if (HasVSMaterialUniformBuffer() || HasPSMaterialUniformBuffer())
			ubufferCount++;

		// TODO: Optimize
		// Resources count
		uint32_t samplerCount = 0, textureCount = 0, textureDescriptorCount = 0;
		for (auto resource : m_Resources)
		{
			auto vk_Resource = dynamic_cast<VulkanShaderResourceDeclaration*>(resource);
			switch (vk_Resource->GetType())
			{
				case VulkanShaderResourceDeclaration::Type::SampledImage:
					textureCount++;
					textureDescriptorCount++;
					break;

				case VulkanShaderResourceDeclaration::Type::Image:
					textureCount++;
					textureDescriptorCount++;
					break;

				case VulkanShaderResourceDeclaration::Type::Sampler:
					samplerCount++;
					break;
			}
		}

		// Total binding count
		uint32_t bindingCount = ubufferCount + samplerCount;
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings(bindingCount);
		
		// VS Uniform Buffers
		for (auto ubuffer : m_VSUniformBuffers)
		{
			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = ubuffer->GetBinding();
			layoutBinding.descriptorCount = 1;
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding.stageFlags = GetDescriptorStage(ubuffer->GetDomain());

			layoutBindings[index++] = layoutBinding;
		}

		// PS Uniform Buffers
		for (auto ubuffer : m_PSUniformBuffers)
		{
			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = ubuffer->GetBinding();
			layoutBinding.descriptorCount = 1;
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding.stageFlags = GetDescriptorStage(ubuffer->GetDomain());

			layoutBindings[index++] = layoutBinding;
		}

		// Material specific uniform buffers
		if (HasVSMaterialUniformBuffer() || HasPSMaterialUniformBuffer()) 
		{
			ShaderUniformBufferDeclaration* ubuffer;
			if (HasVSMaterialUniformBuffer())
				ubuffer = m_VSMaterialUniformBuffer.get();
			else 
				ubuffer = m_PSMaterialUniformBuffer.get();

			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = ubuffer->GetBinding();
			layoutBinding.descriptorCount = 1;
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			layoutBinding.stageFlags = GetDescriptorStage(ubuffer->GetDomain());
			layoutBindings[index++] = layoutBinding;
		}
		
		// Samplers
		for (auto resource : m_Resources)
		{
			auto vk_Resource = dynamic_cast<VulkanShaderResourceDeclaration*>(resource);

			if (vk_Resource->GetType() == VulkanShaderResourceDeclaration::Type::Sampler)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.binding = resource->GetBinding();
				layoutBinding.descriptorCount = resource->GetCount();
				layoutBinding.descriptorType = GetDescriptorType(vk_Resource->GetType());
				layoutBinding.stageFlags = GetDescriptorStage(resource->GetDomain());

				// Set the immutable samplers here, these are default
				// samplers used through all the application
				layoutBinding.pImmutableSamplers = m_Samplers.data();
				
				layoutBindings[index++] = layoutBinding;
			}
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
			&m_GlobalDescriptorSetLayout
		));

		// If has descriptors, create them
		if (bindingCount > 0)
		{
			// Allocate the Global Descriptor Set
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pSetLayouts = &m_GlobalDescriptorSetLayout;
			allocInfo.descriptorSetCount = 1;
			allocInfo.descriptorPool = vk_Context->GetDescriptorPool();

			VK_CHECK_RESULT(vkAllocateDescriptorSets(
				vk_Device->GetLogicalDevice(),
				&allocInfo,
				&m_GlobalDescriptorSet
			));
		}

		// For each descriptor set, process all textures in the set
		for (uint32_t i = 1; i < (textureDescriptorCount + 1); i++)
		{
			std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

			// Process bindings for this set
			for (auto resource : m_Resources)
			{
				auto vk_Resource = dynamic_cast<VulkanShaderResourceDeclaration*>(resource);

				// Process only resources for this set, and only images
				if (vk_Resource->GetSet() == i &&
				   (vk_Resource->GetType() == VulkanShaderResourceDeclaration::Type::Image ||
					vk_Resource->GetType() == VulkanShaderResourceDeclaration::Type::SampledImage))
				{
					VkDescriptorSetLayoutBinding layoutBinding = {};
					layoutBinding.binding = resource->GetBinding();
					layoutBinding.descriptorCount = resource->GetCount();
					layoutBinding.descriptorType = GetDescriptorType(vk_Resource->GetType());
					layoutBinding.stageFlags = GetDescriptorStage(resource->GetDomain());

					layoutBindings.push_back(layoutBinding);
				}
			}

			descriptorSetLayoutInfo.pBindings = layoutBindings.data();
			descriptorSetLayoutInfo.bindingCount = layoutBindings.size();

			VkDescriptorSetLayout layout;
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(
				vk_Device->GetLogicalDevice(),
				&descriptorSetLayoutInfo,
				nullptr,
				&layout
			));

			m_TextureDescriptorSetLayouts.push_back(layout);

			// Allocate the Texture Descriptor Set
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pSetLayouts = &layout;
			allocInfo.descriptorSetCount = 1;
			allocInfo.descriptorPool = vk_Context->GetDescriptorPool();
			
			VkDescriptorSet set;
			VK_CHECK_RESULT(vkAllocateDescriptorSets(
				vk_Device->GetLogicalDevice(),
				&allocInfo,
				&set
			));

			m_TextureDescriptorSets.push_back(set);
		}

		// Update Descriptor Sets
		UpdateDescriptorSets();
	}

	void VulkanShader::CreatePipelineLayout()
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();

		std::vector<VkDescriptorSetLayout> layouts;
		layouts.push_back(m_GlobalDescriptorSetLayout);
		layouts.insert(
			layouts.end(),
			m_TextureDescriptorSetLayouts.begin(),
			m_TextureDescriptorSetLayouts.end()
		);

		// TODO: Set descriptor layouts & push constants
		VkPipelineLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pSetLayouts = layouts.data();
		layoutInfo.setLayoutCount = layouts.size();

		VK_CHECK_RESULT(vkCreatePipelineLayout(
			vk_Device->GetLogicalDevice(),
			&layoutInfo,
			nullptr,
			&m_PipelineLayout
		));
	}

	const VkDescriptorSet& VulkanShader::CreateDescriptorSetForTexture(
		VulkanShaderResourceDeclaration* resource,
		const Ref<Texture>& texture
	)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();
		auto vk_Resource = dynamic_cast<VulkanShaderResourceDeclaration*>(resource);

		// Create and save the descriptor in the pool
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pSetLayouts = &m_TextureDescriptorSetLayouts[vk_Resource->GetSet() - 1]; // TODO: Remove the '-1'
		allocInfo.descriptorSetCount = 1;
		allocInfo.descriptorPool = vk_Context->GetDescriptorPool();

		VkDescriptorSet set;
		VK_CHECK_RESULT(vkAllocateDescriptorSets(
			vk_Device->GetLogicalDevice(),
			&allocInfo,
			&set
		));

		m_TexturePool[texture] = set;

		return set;
	}

	void VulkanShader::UpdateDescriptorSet(VulkanShaderResourceDeclaration* resource, const std::vector<Ref<Texture>>& textures, VkDescriptorSet set)
	{
		// Sampler Descriptors doesn't need to be updated
		if (resource->GetType() != VulkanShaderResourceDeclaration::Type::Sampler)
		{
			VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
			auto& vk_Device = vk_Context->GetDevice();
				
			// Loop through them, to get descriptor infos
			std::vector<VkDescriptorImageInfo> imageInfos(resource->GetCount());
			for (uint32_t i = 0; i < resource->GetCount(); i++)
			{
				// Cast to Vulkan Texture, so we can get descriptor info (specific to VK)
				auto vk_Texture = dynamic_cast<VulkanTexture2D*>(textures[i].get());
				VkDescriptorImageInfo info = vk_Texture->GetDescriptorInfo();
				imageInfos[i] = info;
			}

			VkWriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = set;
			writeDescriptorSet.dstBinding = resource->GetBinding();
			writeDescriptorSet.descriptorType = GetDescriptorType(resource->GetType());
			writeDescriptorSet.descriptorCount = resource->GetCount();					
			writeDescriptorSet.pImageInfo = imageInfos.data();

			vkUpdateDescriptorSets(vk_Device->GetLogicalDevice(), 1, &writeDescriptorSet, 0, nullptr);
		}
	}

	void VulkanShader::UpdateDescriptorSet(VulkanShaderUniformBufferDeclaration* ubuffer)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();

		auto vk_Buffer = dynamic_cast<VulkanUniformBuffer*>(m_UniformBuffers[ubuffer->GetName()].get());
			
		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = m_GlobalDescriptorSet;
		writeDescriptorSet.dstBinding = ubuffer->GetBinding();
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.pBufferInfo = &vk_Buffer->GetDescriptorInfo();

		vkUpdateDescriptorSets(vk_Device->GetLogicalDevice(), 1, &writeDescriptorSet, 0, nullptr);
	}

	void VulkanShader::UpdateDescriptorSets()
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();

		// The buffers will be created on shader creation, so update his descriptors here
		std::vector<VkWriteDescriptorSet> writeDescriptorSets;

		// VS Uniform Buffer
		std::vector<VkDescriptorBufferInfo> vsBufferInfos(m_VSUniformBuffers.size());
		for (uint32_t i = 0; i < m_VSUniformBuffers.size(); i++)
		{
			auto decl = m_VSUniformBuffers[i];
			auto ubuffer = m_UniformBuffers[decl->GetName()];
			auto vk_Buffer = dynamic_cast<VulkanUniformBuffer*>(ubuffer.get());
			
			VkWriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = m_GlobalDescriptorSet;
			writeDescriptorSet.dstBinding = decl->GetBinding();
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.descriptorCount = 1;

			vsBufferInfos[i] = vk_Buffer->GetDescriptorInfo();
			writeDescriptorSet.pBufferInfo = &vsBufferInfos[i];

			writeDescriptorSets.push_back(writeDescriptorSet);
		}
		
		// PS Uniform Buffers
		std::vector<VkDescriptorBufferInfo> psBufferInfos(m_PSUniformBuffers.size());
		for (uint32_t i = 0; i < m_PSUniformBuffers.size(); i++)
		{
			auto decl = m_PSUniformBuffers[i];
			auto ubuffer = m_UniformBuffers[decl->GetName()];
			auto vk_Buffer = dynamic_cast<VulkanUniformBuffer*>(ubuffer.get());
			
			VkWriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = m_GlobalDescriptorSet;
			writeDescriptorSet.dstBinding = decl->GetBinding();
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.descriptorCount = 1;

			psBufferInfos[i] = vk_Buffer->GetDescriptorInfo();
			writeDescriptorSet.pBufferInfo = &psBufferInfos[i];

			writeDescriptorSets.push_back(writeDescriptorSet);
		}

		// Material specific uniform buffers
		if (HasVSMaterialUniformBuffer() || HasPSMaterialUniformBuffer()) 
		{
			ShaderUniformBufferDeclaration* ubuffer;
			if (HasVSMaterialUniformBuffer())
				ubuffer = m_VSMaterialUniformBuffer.get();
			else 
				ubuffer = m_PSMaterialUniformBuffer.get();

			auto vk_Buffer = dynamic_cast<VulkanUniformBuffer*>(m_MaterialUniformBuffer.get());
			
			VkWriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = m_GlobalDescriptorSet;
			writeDescriptorSet.dstBinding = ubuffer->GetBinding();
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.pBufferInfo = &vk_Buffer->GetDescriptorInfo();

			writeDescriptorSets.push_back(writeDescriptorSet);
		}

		// Resources
		std::vector<VkDescriptorImageInfo> imageInfos; // IMPORTANT! SHOULD SUPPORT INFOS FOR EACH DESCRIPTOR
		for (uint32_t set = 0; set < m_TextureDescriptorSets.size(); set++)
		{
			for (auto resource : m_Resources)
			{
				auto vk_Resource = dynamic_cast<VulkanShaderResourceDeclaration*>(resource);

				// Update only descriptors in this set
				// Sampler Descriptors doesn't need to be updated
				if (vk_Resource->GetSet() == (set + 1) &&		// TODO: Remove the '+1'
					vk_Resource->GetType() != VulkanShaderResourceDeclaration::Type::Sampler)
				{
					// Get all textures in the array
					auto& textures = m_Textures[resource->GetName()];

					// Loop through them, to get descriptor infos
					imageInfos.resize(resource->GetCount());
					for (uint32_t i = 0; i < resource->GetCount(); i++)
					{
						// Cast to Vulkan Texture, so we can get descriptor info (specific to VK)
						auto vk_Texture = dynamic_cast<VulkanTexture2D*>(textures[i].get());
						VkDescriptorImageInfo info = vk_Texture->GetDescriptorInfo();
						imageInfos[i] = info;
					}

					VkWriteDescriptorSet writeDescriptorSet = {};
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSet.dstSet = m_TextureDescriptorSets[set];
					writeDescriptorSet.dstBinding = resource->GetBinding();
					writeDescriptorSet.descriptorType = GetDescriptorType(vk_Resource->GetType());
					writeDescriptorSet.descriptorCount = resource->GetCount();
					writeDescriptorSet.pImageInfo = imageInfos.data();

					writeDescriptorSets.push_back(writeDescriptorSet);
				}
			}
		}

		vkUpdateDescriptorSets(
			vk_Device->GetLogicalDevice(),
			writeDescriptorSets.size(),
			writeDescriptorSets.data(),
			0,
			nullptr
		);
	}

	void VulkanShader::GetShaderResources(const std::vector<uint32_t>& spirv, ShaderDomain domain)
	{
		spirv_cross::Compiler compiler(spirv);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		// samplerCube, sampler2D
		for (const auto& image : resources.sampled_images)
		{	
			unsigned set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
			unsigned binding = compiler.get_decoration(image.id, spv::DecorationBinding);
				
			// Has at least 1 texture
			uint32_t count = 1;

			// Check if is an array and his size
			const auto& spvType = compiler.get_type(image.type_id);
			if (spvType.array.size() > 0)
			{
				count = 0;
				
				for (uint32_t i = 0; i < spvType.array.size(); i++)
				{
					count += spvType.array[i];
				}
			}

			// Type here is SampledImage
			auto type = VulkanShaderResourceDeclaration::Type::SampledImage;

			// Try to get the dimension of this texture
			auto dimension = VulkanShaderResourceDeclaration::Dimension::None;
			const auto& spvBaseType = compiler.get_type(image.base_type_id);
			switch (spvBaseType.image.dim)
			{
				case spv::Dim2D:	dimension = VulkanShaderResourceDeclaration::Dimension::Texture2D;   break;
				case spv::DimCube:	dimension = VulkanShaderResourceDeclaration::Dimension::TextureCube; break;
			}

			// Create the Resource Declaration
			auto resourceDecl = new VulkanShaderResourceDeclaration(image.name, set, binding, domain, type, dimension, count);
			m_Resources.push_back(resourceDecl);
		}

		// texture2D
		for (const auto& image : resources.separate_images)
		{
			unsigned set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
			unsigned binding = compiler.get_decoration(image.id, spv::DecorationBinding);
				
			// Has at least 1 resource
			uint32_t count = 1;

			// Check if is an array and his size
			const auto& spvType = compiler.get_type(image.type_id);
			if (spvType.array.size() > 0)
			{
				count = 0;
				
				for (uint32_t i = 0; i < spvType.array.size(); i++)
				{
					count += spvType.array[i];
				}
			}

			// Type here is Image
			auto type = VulkanShaderResourceDeclaration::Type::Image;

			// Try to get the dimension of this texture
			auto dimension = VulkanShaderResourceDeclaration::Dimension::None;
			const auto& spvBaseType = compiler.get_type(image.base_type_id);
			switch (spvBaseType.image.dim)
			{
				case spv::Dim2D:	dimension = VulkanShaderResourceDeclaration::Dimension::Texture2D;   break;
				case spv::DimCube:	dimension = VulkanShaderResourceDeclaration::Dimension::TextureCube; break;
			}

			// Create the Resource Declaration
			auto resourceDecl = new VulkanShaderResourceDeclaration(image.name, set, binding, domain, type, dimension, count);
			m_Resources.push_back(resourceDecl);
		}

		// sampler
		for (const auto& sampler : resources.separate_samplers)
		{
			unsigned set = compiler.get_decoration(sampler.id, spv::DecorationDescriptorSet);
			unsigned binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
				
			// Has at least 1 resource
			uint32_t count = 1;

			// Check if is an array and his size
			const auto& spvType = compiler.get_type(sampler.type_id);
			if (spvType.array.size() > 0)
			{
				count = 0;
				
				for (uint32_t i = 0; i < spvType.array.size(); i++)
				{
					count += spvType.array[i];
				}
			}

			// Type here is Sampler
			auto type = VulkanShaderResourceDeclaration::Type::Sampler;

			// Sampler has no dimension
			auto dimension = VulkanShaderResourceDeclaration::Dimension::None;

			// Create the Resource Declaration
			auto resourceDecl = new VulkanShaderResourceDeclaration(sampler.name, set, binding, domain, type, dimension, count);
			m_Resources.push_back(resourceDecl);
		}

		// uniform buffer
		for (const auto& ubuffer : resources.uniform_buffers)
		{
			unsigned binding = compiler.get_decoration(ubuffer.id, spv::DecorationBinding);
			
			// Uses the name queried using the id, as it returns the variable name rather than block name
			auto name = compiler.get_name(ubuffer.id);

			// Create the Uniform Buffer Declaration
			auto bufferDecl = CreateScope<VulkanShaderUniformBufferDeclaration>(name, binding, domain);

			// Loop through all members of the uniform buffer, and create a Uniform Declaration for each them
			uint32_t index = 0;
			const auto& spvBaseType = compiler.get_type(ubuffer.base_type_id);
			for (auto typeId : spvBaseType.member_types)
			{
				auto memberType = compiler.get_type(typeId);
				auto memberName = compiler.get_member_name(spvBaseType.self, index);

				// Get type
				auto type = VulkanShaderUniformDeclaration::Type::None;
				switch (memberType.basetype)
				{
				case spirv_cross::SPIRType::Int:
					type = VulkanShaderUniformDeclaration::Type::Int32;
					break;

				case spirv_cross::SPIRType::Float:
					if (memberType.vecsize == 1)
						type = VulkanShaderUniformDeclaration::Type::Float32;
					else if (memberType.vecsize == 2)
						type = VulkanShaderUniformDeclaration::Type::Vec2;
					else if (memberType.vecsize == 3)
						type = VulkanShaderUniformDeclaration::Type::Vec3;
					else if (memberType.vecsize == 2)
						type = VulkanShaderUniformDeclaration::Type::Vec2;
					else if (memberType.vecsize == 4 && memberType.columns == 1)
						type = VulkanShaderUniformDeclaration::Type::Vec4;
					else if (memberType.vecsize == 4 && memberType.columns == 3)
						type = VulkanShaderUniformDeclaration::Type::Mat3;
					else if (memberType.vecsize == 4 && memberType.columns == 4)
						type = VulkanShaderUniformDeclaration::Type::Mat4;
					break;
				}

				// Has at least 1 uniform
				uint32_t count = 1;

				// Check if is an array and his size
				if (memberType.array.size() > 0)
				{
					// Support only vectors now
					count = memberType.array[0];
				}

				auto uniform = new VulkanShaderUniformDeclaration(memberName, domain, type, count);
				bufferDecl->PushUniform(uniform);

				index++;
			}

			// TODO: Define this string as static
			if (ubuffer.name == "MaterialData")
			{
				switch (domain)
				{
					case Hazel::ShaderDomain::Vertex:
						m_VSMaterialUniformBuffer = std::move(bufferDecl);
						break;

					case Hazel::ShaderDomain::Pixel: 
						m_PSMaterialUniformBuffer = std::move(bufferDecl);
						break;
				}
			}
			else
			{
				auto decl = bufferDecl.release();
				switch (domain)
				{
					case Hazel::ShaderDomain::Vertex: m_VSUniformBuffers.push_back(decl); break;
					case Hazel::ShaderDomain::Pixel:  m_PSUniformBuffers.push_back(decl); break;
				}
			}
		}
	}

	void VulkanShader::GenerateShaderResources()
	{
		// VS Uniform Buffers
		for (auto ubuffer : m_VSUniformBuffers)
		{
			m_UniformBuffers[ubuffer->GetName()] = UniformBuffer::Create(ubuffer->GetSize());
		}

		// PS Uniform Buffers
		for (auto ubuffer : m_PSUniformBuffers)
		{
			m_UniformBuffers[ubuffer->GetName()] = UniformBuffer::Create(ubuffer->GetSize());
		}

		// Material Uniform Buffer
		if (HasVSMaterialUniformBuffer() || HasPSMaterialUniformBuffer())
		{
			VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
			auto& vk_Device = vk_Context->GetDevice();

			ShaderUniformBufferDeclaration* ubuffer;
			if (HasVSMaterialUniformBuffer())
				ubuffer = m_VSMaterialUniformBuffer.get();
			else
				ubuffer = m_PSMaterialUniformBuffer.get();

			uint32_t minUniformBufferAlignment = vk_Device->GetProperties().limits.minUniformBufferOffsetAlignment;
			m_MaterialUniformBufferAlignment = ubuffer->GetSize();
			if (minUniformBufferAlignment > 0)
			{
				m_MaterialUniformBufferAlignment =
					(m_MaterialUniformBufferAlignment + minUniformBufferAlignment - 1) &
					~(minUniformBufferAlignment - 1);
			}

			// TODO: Should not be VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			m_MaterialUniformBuffer = CreateScope<VulkanUniformBuffer>(m_MaterialUniformBufferAlignment * MAX_MAT_INSTANCES);
		}

		// Textures
		for (auto resource : m_Resources)
		{
			auto vk_Resource = dynamic_cast<VulkanShaderResourceDeclaration*>(resource);
			
			// Only process Images and SampledImages
			if (vk_Resource->GetType() == VulkanShaderResourceDeclaration::Type::Image ||
				vk_Resource->GetType() == VulkanShaderResourceDeclaration::Type::SampledImage)
			{
				// TODO: Set the default magenta texture /////////////////////////////
				uint32_t content = 0xFFFF00FF;
				Ref<Texture2D> defaultTex = Texture2D::Create(&content, 1, 1, 4);
				//////////////////////////////////////////////////////////////////////

				std::vector<Ref<Texture>> textures(resource->GetCount());
				
				for (uint32_t i = 0; i < resource->GetCount(); i++)
				{
					textures[i] = (const Ref<Texture>&)defaultTex;
				}

				m_Textures[resource->GetName()] = textures;
			}
		}
	}
	
	VkDescriptorType VulkanShader::GetDescriptorType(VulkanShaderResourceDeclaration::Type type)
	{
		switch (type)
		{
			case Hazel::VulkanShaderResourceDeclaration::Type::None:
			case Hazel::VulkanShaderResourceDeclaration::Type::Image:
				return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		
			case Hazel::VulkanShaderResourceDeclaration::Type::SampledImage:
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

			case Hazel::VulkanShaderResourceDeclaration::Type::Sampler:
				return VK_DESCRIPTOR_TYPE_SAMPLER;
		}
	}

	VkShaderStageFlags VulkanShader::GetDescriptorStage(ShaderDomain domain)
	{
		switch (domain)
		{
			case ShaderDomain::None:
			case ShaderDomain::Vertex:
				return VK_SHADER_STAGE_VERTEX_BIT;
		
			case ShaderDomain::Pixel:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
		}
	}

}