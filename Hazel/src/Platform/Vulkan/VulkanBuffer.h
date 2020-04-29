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
		VulkanIndexBuffer(uint32_t* data, uint32_t count);
		virtual ~VulkanIndexBuffer() = default;

		virtual void Bind() const override {};
		virtual void Unbind() const override {};

		virtual uint32_t GetCount() const override { return m_Count; }

		const VkBuffer& GetDeviceBuffer() const { return m_DeviceBuffer; }

	private:
		uint32_t m_Count;

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

		const VkBuffer& GetDeviceBuffer() const { return m_DeviceBuffer; }
		VkDescriptorBufferInfo GetDescriptorInfo() const { return m_DescriptorInfo; }

	private:
		uint32_t m_Size;
		
		VkBuffer m_DeviceBuffer;
		VkDeviceMemory m_DeviceMemory;

		VkDescriptorBufferInfo m_DescriptorInfo;
	};
	
}
