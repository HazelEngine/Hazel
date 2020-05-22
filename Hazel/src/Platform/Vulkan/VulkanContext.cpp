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

		// Depth/Stencil resources creation
		CreateDepthStencilResources();

		// RenderPass
		CreateRenderPass();

		// Framebuffer
		CreateFramebuffers();

		// CommandPool and Buffers
		CreateCommandPool();
		CreateCommandBuffers();

		// Create DescriptorPool
		CreateDescriptorPool();

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

	void VulkanContext::CreateDepthStencilResources()
	{
		m_DepthStencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

		// Create the image

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.mipLevels = 1;
		imageInfo.format = m_DepthStencilFormat;
		imageInfo.arrayLayers = 1;
		imageInfo.extent.width = m_SwapchainWidth;
		imageInfo.extent.height = m_SwapchainHeight;
		imageInfo.extent.depth = 1;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VK_CHECK_RESULT(vkCreateImage(
			m_Device->GetLogicalDevice(),
			&imageInfo,
			nullptr,
			&m_DepthStencilBuffer.image
		));

		// Allocate memory for image

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(m_Device->GetLogicalDevice(), m_DepthStencilBuffer.image, &memReqs);

		m_MemoryAllocator->Allocate(
			memReqs,
			&m_DepthStencilBufferMemory,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		vkBindImageMemory(
			m_Device->GetLogicalDevice(),
			m_DepthStencilBuffer.image,
			m_DepthStencilBufferMemory,
			0
		);

		// Create the ImageView

		VkImageViewCreateInfo depthAttachmentView = {};
		depthAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthAttachmentView.format = m_DepthStencilFormat;
		depthAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthAttachmentView.subresourceRange.levelCount = 1;
		depthAttachmentView.subresourceRange.layerCount = 1;
		depthAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthAttachmentView.image = m_DepthStencilBuffer.image;

		VK_CHECK_RESULT(vkCreateImageView(
			m_Device->GetLogicalDevice(),
			&depthAttachmentView,
			nullptr, 
			&m_DepthStencilBuffer.view
		));
	}

	void VulkanContext::CreateRenderPass()
	{
		// Color Attachment

		VkAttachmentDescription colorAttachmentDesc = {};
		colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Sample Count?
		colorAttachmentDesc.format = m_Swapchain->GetFormat();
		colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Depth Attachment

		VkAttachmentDescription depthAttachmentDesc = {};
		depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Sample Count?
		depthAttachmentDesc.format = m_DepthStencilFormat;
		depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Subpass

		VkSubpassDescription subpassDesc = {};
		subpassDesc.inputAttachmentCount = 0;
		subpassDesc.pColorAttachments = &colorAttachmentRef;
		subpassDesc.colorAttachmentCount = 1;
		subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;
		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		// Create RenderPass

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachmentDesc, depthAttachmentDesc };
		
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
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
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.width = m_SwapchainWidth;
		framebufferInfo.height = m_SwapchainHeight;
		framebufferInfo.layers = 1;

		std::array<VkImageView, 2> attachments;
		for (uint32_t i = 0; i < m_Swapchain->GetBufferCount(); i++)
		{
			attachments = {
				m_Swapchain->GetBuffer(i).view,
				m_DepthStencilBuffer.view
			};

			framebufferInfo.pAttachments = attachments.data();

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

	void VulkanContext::CreateDescriptorPool()
	{
		VkDescriptorPoolSize poolSizes[5] = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = MAX_DESCRIPTOR_UBUFFER;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizes[1].descriptorCount = MAX_DESCRIPTOR_UBUFFER_DYNAMIC;

		poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		poolSizes[2].descriptorCount = MAX_DESCRIPTOR_SAMPLED_IMAGE;

		poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[3].descriptorCount = MAX_DESCRIPTOR_COMBINED_IMAGE_SAMPLER;

		poolSizes[4].type = VK_DESCRIPTOR_TYPE_SAMPLER;
		poolSizes[4].descriptorCount = MAX_DESCRIPTOR_SAMPLER;

		VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.maxSets = MAX_DESCRIPTOR_SETS;
		descriptorPoolInfo.pPoolSizes = poolSizes;
		descriptorPoolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize);

		VK_CHECK_RESULT(vkCreateDescriptorPool(
			m_Device->GetLogicalDevice(),
			&descriptorPoolInfo,
			nullptr,
			&m_DescriptorPool
		));
	}

	void VulkanContext::Cleanup()
	{
		vkDestroyDescriptorPool(m_Device->GetLogicalDevice(), m_DescriptorPool, nullptr);

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