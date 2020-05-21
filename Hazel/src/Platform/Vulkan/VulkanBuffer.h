#pragma once

#include <Hazel/Renderer/Buffer.h>

#include <vulkan/vulkan.hpp>

namespace Hazel {

	class HAZEL_API VulkanVertexBuffer : public VertexBuffer
	{
		
	public:
		VulkanVertexBuffer(uint32_t size);
		VulkanVertexBuffer(void* data, uint32_t size);
		virtual ~VulkanVertexBuffer() = default;

		virtual void Bind() const override {};
		virtual void Unbind() const override {};

		virtual void* Map() override;
		virtual void Unmap(uint32_t size) override;

		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

		const VkBuffer& GetDeviceBuffer() const { return m_DeviceBuffer; }

	private:
		BufferLayout m_Layout;
		uint32_t m_Size;

		VkBuffer m_StagingBuffer;
		VkDeviceMemory m_StagingMemory;
		
		VkBuffer m_DeviceBuffer;
		VkDeviceMemory m_DeviceMemory;
	};

	class HAZEL_API VulkanIndexBuffer : public IndexBuffer
	{
		
	public:
		VulkanIndexBuffer(void* data, uint32_t size);
		virtual ~VulkanIndexBuffer() = default;

		virtual void Bind() const override {};
		virtual void Unbind() const override {};

		virtual uint32_t GetSize() const override { return m_Size; }

		const VkBuffer& GetDeviceBuffer() const { return m_DeviceBuffer; }

	private:
		uint32_t m_Size;

		VkBuffer m_DeviceBuffer;
		VkDeviceMemory m_DeviceMemory;
	};

	class HAZEL_API VulkanUniformBuffer : public UniformBuffer
	{
		
	public:
		VulkanUniformBuffer(uint32_t size);
		VulkanUniformBuffer(void* data, uint32_t size);
		virtual ~VulkanUniformBuffer() = default;

		virtual void Bind() const override {}
		virtual void Unbind() const override {}

		virtual void* Map() override;
		virtual void Unmap(uint32_t size) override;
		virtual void Unmap(uint32_t offset, uint32_t size) override;

		uint32_t GetSize() const { return m_Size; }

		const VkBuffer& GetDeviceBuffer() const { return m_DeviceBuffer; }
		VkDescriptorBufferInfo GetDescriptorInfo() const { return m_DescriptorInfo; }

	private:
		uint32_t m_Size;
		
		VkBuffer m_DeviceBuffer;
		VkDeviceMemory m_DeviceMemory;

		VkDescriptorBufferInfo m_DescriptorInfo;
	};
	
	inline VkFormat ShaderDataTypeToVulkanBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case Hazel::ShaderDataType::Float:		return VK_FORMAT_R32_SFLOAT;
		case Hazel::ShaderDataType::Float2:		return VK_FORMAT_R32G32_SFLOAT;
		case Hazel::ShaderDataType::Float3:		return VK_FORMAT_R32G32B32_SFLOAT;
		case Hazel::ShaderDataType::Float4:		return VK_FORMAT_R32G32B32A32_SFLOAT;
		case Hazel::ShaderDataType::Int:		return VK_FORMAT_R32_SINT;
		case Hazel::ShaderDataType::Int2:		return VK_FORMAT_R32G32_SINT;
		case Hazel::ShaderDataType::Int3:		return VK_FORMAT_R32G32B32_SINT;
		case Hazel::ShaderDataType::Int4:		return VK_FORMAT_R32G32B32A32_SINT;
		}
	}

}
