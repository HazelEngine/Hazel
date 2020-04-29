#pragma once

#include <Hazel/Renderer/Shader.h>
#include <Hazel/Renderer/Buffer.h>

#include <vulkan/vulkan.hpp>

namespace Hazel {

	class HAZEL_API VulkanShader : public Shader
	{
	public:
		struct ShaderResource
		{
			uint32_t Binding;
			std::string Name;
			VkShaderStageFlags StageFlags;
			VkDescriptorType DescriptorType;
			uint32_t DescriptorCount;

			uint32_t Size;

			bool IsBuffer = false;
			bool IsImage = false;
			bool IsSampler = false;
		};

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

		virtual void SetUniformBuffer(const std::string& name, void* data, uint32_t size) override;

		virtual void BindTexture(const std::string& name, const Ref<Texture2D>& texture) override;
		virtual Ref<Texture2D> GetTexture(const std::string& name) const override;

		virtual const std::string& GetName() const override { return m_Name; }

		VkShaderModule GetVertexShaderModule() const { return m_VertexShaderModule; }
		VkShaderModule GetFragmentShaderModule() const { return m_FragmentShaderModule; }

		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
		const VkDescriptorSet& GetDescriptorSet() const { return m_DescriptorSet; }

	private:
		void CreateSamplers();
		void CreateDescriptorSet();
		void CreatePipelineLayout();

		void GetShaderResources(const std::vector<uint32_t>& spirv, VkShaderStageFlags stage);
		void GenerateShaderResources();

	private:
		std::string m_Name;

		std::vector<ShaderResource> m_ShaderResources;
		std::unordered_map<std::string, Ref<UniformBuffer>> m_UniformBuffers;
		std::unordered_map<std::string, Ref<Texture2D>> m_Textures;

		VkShaderModule m_VertexShaderModule = VK_NULL_HANDLE;
		VkShaderModule m_FragmentShaderModule = VK_NULL_HANDLE;

		// TODO: Put inside Texture or Sample class (static)
		std::vector<VkSampler> m_Samplers;

		VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;

		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	};

}