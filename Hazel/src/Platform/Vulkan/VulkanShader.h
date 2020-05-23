#pragma once

#include <Hazel/Renderer/Shader.h>
#include <Hazel/Renderer/Buffer.h>

#include <Platform/Vulkan/VulkanShaderUniform.h>

#include <vulkan/vulkan.hpp>

#define MAX_MAT_INSTANCES 20

namespace spirv_cross {
	class Compiler;
	struct SPIRType;
}

namespace Hazel {

	class HAZEL_API VulkanShader : public Shader
	{
	public:
		VulkanShader(const ShaderCreateInfo& info);
		virtual ~VulkanShader() = default;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;

		virtual void SetFloat(const std::string & name, float value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;

		virtual void SetMat3(const std::string& name, const glm::mat3& value) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

		virtual const ShaderUniformBufferList GetVSUniformBuffers() const override { return m_VSUniformBuffers; }
		virtual const ShaderUniformBufferList GetPSUniformBuffers() const override { return m_PSUniformBuffers; }
		virtual const ShaderResourceList GetResources() const override { return m_Resources; }

		virtual void SetUniformBuffer(const std::string& name, void* data, uint32_t size) override;
		virtual void SetUniformBufferParam(const std::string& name, const std::string& param, void* data, uint32_t size) override;

		virtual void BindTexture(const std::string& name, const Ref<Texture>& texture) override;
		virtual void BindTexture(const std::string& name, uint32_t index, const Ref<Texture>& texture) override;
		
		virtual void BindTextureToPool(const std::string& name, const Ref<Texture>& texture) override;
		
		virtual Ref<Texture> GetTexture(const std::string& name) const override;
		virtual Ref<Texture> GetTexture(const std::string& name, uint32_t index) const override;

		// Specific to Material uniforms
		virtual bool HasVSMaterialUniformBuffer() const override { return (bool)m_VSMaterialUniformBuffer; }
		virtual bool HasPSMaterialUniformBuffer() const override { return (bool)m_PSMaterialUniformBuffer; }
		virtual const ShaderUniformBufferDeclaration& GetVSMaterialUniformBuffer() const override { return *m_VSMaterialUniformBuffer; }
		virtual const ShaderUniformBufferDeclaration& GetPSMaterialUniformBuffer() const override { return *m_PSMaterialUniformBuffer; }
		virtual void SetMaterialUniformBuffer(Buffer buffer, uint32_t materialIndex) override;
		
		virtual uint32_t GetMaterialCount() const override { return m_MaterialCount; }
		virtual void SetMaterialCount(uint32_t count) override { m_MaterialCount = count; }

		virtual uint32_t GetMaterialUniformBufferAlignment() const { return m_MaterialUniformBufferAlignment; }

		uint32_t GetTextureDescriptorSetIndex(const std::string& name) const;
		const VkDescriptorSet& GetTexturePoolDescriptorSet(const Ref<Texture>& texture) { return m_TexturePool[texture]; }

		virtual const std::string& GetName() const override { return m_Name; }

		VkShaderModule GetVertexShaderModule() const { return m_VertexShaderModule; }
		VkShaderModule GetFragmentShaderModule() const { return m_FragmentShaderModule; }

		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

		const VkDescriptorSet& GetGlobalDescriptorSet() const { return m_GlobalDescriptorSet; }
		const std::vector<VkDescriptorSet>& GetTextureDescriptorSets() const { return m_TextureDescriptorSets; }

	private:
		void CreateSamplers();
		void CreateDescriptorSets();
		void CreatePipelineLayout();

		const VkDescriptorSet& CreateDescriptorSetForTexture(
			VulkanShaderResourceDeclaration* resource,
			const Ref<Texture>& texture
		);

		void UpdateDescriptorSet(VulkanShaderResourceDeclaration* resource, const std::vector<Ref<Texture>>& textures, VkDescriptorSet set);
		void UpdateDescriptorSet(VulkanShaderUniformBufferDeclaration* ubuffer);
		void UpdateDescriptorSets();

		void GetShaderResources(const std::vector<uint32_t>& spirv, ShaderDomain domain);
		void GenerateShaderResources();

		ShaderStruct* ParseShaderStruct(spirv_cross::Compiler& compiler, spirv_cross::SPIRType type, const std::string& name, ShaderDomain domain);

		VkDescriptorType GetDescriptorType(VulkanShaderResourceDeclaration::Type type);
		VkShaderStageFlags GetDescriptorStage(ShaderDomain domain);

	private:
		std::string m_Name;

		// Shader resource declarations
		ShaderUniformBufferList m_VSUniformBuffers;
		ShaderUniformBufferList m_PSUniformBuffers;
		ShaderResourceList m_Resources;

		// Specific material resource declarations
		Scope<VulkanShaderUniformBufferDeclaration> m_VSMaterialUniformBuffer;
		Scope<VulkanShaderUniformBufferDeclaration> m_PSMaterialUniformBuffer;
		Scope<UniformBuffer> m_MaterialUniformBuffer;
		uint32_t m_MaterialUniformBufferAlignment;
		uint32_t m_MaterialCount;

		// Shader resources
		std::unordered_map<std::string, Ref<UniformBuffer>> m_UniformBuffers;
		std::unordered_map<std::string, std::vector<Ref<Texture>>> m_Textures;

		// Store all textures that have been binded to shader and his description
		std::unordered_map<Ref<Texture>, VkDescriptorSet> m_TexturePool;

		VkShaderModule m_VertexShaderModule = VK_NULL_HANDLE;
		VkShaderModule m_FragmentShaderModule = VK_NULL_HANDLE;

		// TODO: Put inside Texture or Sample class (static)
		std::vector<VkSampler> m_Samplers;

		// This descriptor set is used for all global data (with set = 0)
		VkDescriptorSet m_GlobalDescriptorSet = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_GlobalDescriptorSetLayout = VK_NULL_HANDLE;

		// All the textures should have to be binded to these sets (set >= 1)
		std::vector<VkDescriptorSet> m_TextureDescriptorSets;
		std::vector<VkDescriptorSetLayout> m_TextureDescriptorSetLayouts;

		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	};

}