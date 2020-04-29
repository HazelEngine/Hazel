#include "hzpch.h"
#include "VulkanContext.h"

#include "VulkanUtils.h"

// TODO: Remove!
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

std::vector<const char*> s_InstanceExt =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

namespace Hazel {
	
	VulkanContext::VulkanContext(GLFWwindow* window)
		: m_Window(window) {}
	
	VulkanContext::~VulkanContext()
	{
		Cleanup();
	}

	void VulkanContext::Init()
	{
		CreateInstance();

		// Device setup
		m_Device = CreateScope<VulkanDevice>();
		m_Device->Init(m_Instance);
		
		// Memory allocator
		m_MemoryAllocator = CreateScope<VulkanMemoryAllocator>();
		m_MemoryAllocator->Init(m_Device->GetPhysicalDevice(), m_Device->GetLogicalDevice());

		// Swapchain setup
		void* hWnd = glfwGetWin32Window(m_Window);
		void* hInstance = GetModuleHandle(nullptr);
		m_Swapchain = CreateScope<VulkanSwapchain>();
		m_Swapchain->Connect(m_Instance, m_Device->GetPhysicalDevice(), m_Device->GetLogicalDevice());
		m_Swapchain->InitSurface(hInstance, hWnd);
		
		m_SwapchainWidth = 1280;
		m_SwapchainHeight = 720;
		m_Swapchain->Create(&m_SwapchainWidth, &m_SwapchainHeight, true);

		// RenderPass
		CreateRenderPass();

		// Framebuffer
		CreateFramebuffers();

		// CommandPool and Buffers
		CreateCommandPool();
		CreateCommandBuffers();

		// Semaphores
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vkCreateSemaphore(m_Device->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_ImageAcquiredSemaphore);
		vkCreateSemaphore(m_Device->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderingCompleteSemaphore);

		// Frame Fences
		m_FrameFences.resize(m_Swapchain->GetBufferCount());
		for (int i = 0; i < m_Swapchain->GetBufferCount(); ++i)
        {
            VkFenceCreateInfo fenceInfo = {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

            // We need this so we can wait for them on the first try
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            vkCreateFence(m_Device->GetLogicalDevice(), &fenceInfo, nullptr, &m_FrameFences[i]);
		}
	}

	void VulkanContext::Prepare()
	{
		m_Swapchain->AcquireNextImage(m_ImageAcquiredSemaphore, &m_CurrentBackbuffer);

		vkWaitForFences(
			m_Device->GetLogicalDevice(),
			1,
			&m_FrameFences[m_CurrentBackbuffer],
			VK_TRUE,
			UINT64_MAX
		);

        vkResetFences(m_Device->GetLogicalDevice(), 1, &m_FrameFences[m_CurrentBackbuffer]);
	}

	void VulkanContext::SwapBuffers()
	{
		// Submit rendering work to the graphics queue
		const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitSemaphores = &m_ImageAcquiredSemaphore;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_RenderingCompleteSemaphore;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pWaitDstStageMask = &waitDstStageMask;
		submitInfo.pCommandBuffers = &m_DrawCmdBuffers[m_CurrentBackbuffer];
		submitInfo.commandBufferCount = 1;
		
		vkQueueSubmit(m_Device->GetGraphicsQueue(), 1, &submitInfo, m_FrameFences[m_CurrentBackbuffer]);

		m_Swapchain->QueuePresent(
			m_Device->GetGraphicsQueue(),
			m_CurrentBackbuffer,
			m_RenderingCompleteSemaphore
		);
	}

	void VulkanContext::FlushCommandBuffer(VkCommandBuffer cmdBuffer)
	{
		if (cmdBuffer == VK_NULL_HANDLE) return;

		VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		VkFence fence;
		VK_CHECK_RESULT(vkCreateFence(m_Device->GetLogicalDevice(), &fenceInfo, nullptr, &fence));

		// Submit to the queue
		VK_CHECK_RESULT(vkQueueSubmit(m_Device->GetGraphicsQueue(), 1, &submitInfo, fence));
		
		// Wait for the fence to signal that command buffer has finished executing
		VK_CHECK_RESULT(vkWaitForFences(m_Device->GetLogicalDevice(), 1, &fence, VK_TRUE, UINT64_MAX));
		vkDestroyFence(m_Device->GetLogicalDevice(), fence, nullptr);
	}

	VkCommandBuffer VulkanContext::GetCommandBuffer(bool begin)
	{
		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(
			m_Device->GetLogicalDevice(),
			&allocInfo,
			&commandBuffer
		));

		// If requested, also start the new command buffer
		if (begin)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		}

		return commandBuffer;
	}

	void VulkanContext::CreateInstance()
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hazel";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Hazel";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instanceInfo = {};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;

		instanceInfo.enabledExtensionCount = s_InstanceExt.size();
		instanceInfo.ppEnabledExtensionNames = s_InstanceExt.data();

		if (CheckValidationLayerIsPresent())
		{
			const char* layerName = "VK_LAYER_KHRONOS_validation";
			instanceInfo.ppEnabledLayerNames = &layerName;
			instanceInfo.enabledLayerCount = 1;
		}

		VK_CHECK_RESULT(vkCreateInstance(&instanceInfo, nullptr, &m_Instance));
	}

	void VulkanContext::CreateRenderPass()
	{
		VkAttachmentDescription attachmentDesc = {};
		attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Sample Count?
		attachmentDesc.format = m_Swapchain->GetFormat();
		attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		VkAttachmentReference attachmentRef = {};
		attachmentRef.attachment = 0;
		attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDesc = {};
		subpassDesc.inputAttachmentCount = 0;
		subpassDesc.pColorAttachments = &attachmentRef;
		subpassDesc.colorAttachmentCount = 1;
		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = &attachmentDesc;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pSubpasses = &subpassDesc;
		renderPassInfo.subpassCount = 1;

		VK_CHECK_RESULT(vkCreateRenderPass(
			m_Device->GetLogicalDevice(),
			&renderPassInfo,
			nullptr,
			&m_RenderPass
		));
	}

	void VulkanContext::CreateFramebuffers()
	{
		m_Framebuffers.resize(m_Swapchain->GetBufferCount());

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.width = m_SwapchainWidth;
		framebufferInfo.height = m_SwapchainHeight;
		framebufferInfo.layers = 1;

		for (uint32_t i = 0; i < m_Swapchain->GetBufferCount(); i++)
		{
			framebufferInfo.pAttachments = { &m_Swapchain->GetBuffer(i).view };
			VK_CHECK_RESULT(vkCreateFramebuffer(
				m_Device->GetLogicalDevice(),
				&framebufferInfo,
				nullptr,
				&m_Framebuffers[i]
			));
		}
	}

	void VulkanContext::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_Device->GetQueueFamilyIndices().GraphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK_RESULT(vkCreateCommandPool(m_Device->GetLogicalDevice(), &poolInfo, nullptr, &m_CommandPool));
	}

	void VulkanContext::CreateCommandBuffers()
	{
		m_DrawCmdBuffers.resize(m_Swapchain->GetBufferCount());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_DrawCmdBuffers.size();

		VK_CHECK_RESULT(vkAllocateCommandBuffers(
			m_Device->GetLogicalDevice(),
			&allocInfo,
			m_DrawCmdBuffers.data()
		));
	}

	void VulkanContext::Cleanup()
	{
		vkDestroySemaphore(m_Device->GetLogicalDevice(), m_ImageAcquiredSemaphore, nullptr);
		vkDestroySemaphore(m_Device->GetLogicalDevice(), m_RenderingCompleteSemaphore, nullptr);
	
		for (auto fence : m_FrameFences) {
			vkDestroyFence(m_Device->GetLogicalDevice(), fence, nullptr);
		}

		vkDestroyCommandPool(m_Device->GetLogicalDevice(), m_CommandPool, nullptr);

		for (auto framebuffer : m_Framebuffers)
		{
			vkDestroyFramebuffer(m_Device->GetLogicalDevice(), framebuffer, nullptr);
		}

		m_Swapchain->Cleanup();
		m_Device->Cleanup();
		vkDestroyInstance(m_Instance, nullptr);
	}

	bool VulkanContext::CheckValidationLayerIsPresent()
	{
		// The VK_LAYER_KHRONOS_validation contains all current validation functionality.
		// Note that on Android this layer requires at least NDK r20 
		const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
		// Check if this layer is available at instance level
		uint32_t instanceLayerCount;
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
		std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
		bool validationLayerPresent = false;
		for (VkLayerProperties layer : instanceLayerProperties)
		{
			if (strcmp(layer.layerName, validationLayerName) == 0)
			{
				validationLayerPresent = true;
				break;
			}
		}
		if (!validationLayerPresent)
		{
			HZ_CORE_ERROR("Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled")
		}
		return validationLayerPresent;
	}

}