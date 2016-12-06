#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include "Vulkan_Manager.h"

namespace Prometheus {
	namespace Graphics {

		VulkanManager::VulkanManager(Window* window) : m_Window(window) {
			try {
				InitVulkan();
			}
			catch (std::runtime_error e) {
				std::cerr << e.what() << std::endl;
			}
		}

		VulkanManager::~VulkanManager() {

		}

		void VulkanManager::InitVulkan() {
			ApplicationInfo AppInfo;
			AppInfo.pApplicationName = m_Window->getName();
			AppInfo.applicationVersion = M_APP_VERSION;
			AppInfo.pEngineName = M_ENGINE_NAME;
			AppInfo.engineVersion = M_ENGINE_VERSION;
			AppInfo.apiVersion = VK_API_VERSION_1_0;
#ifdef DEBUG_MODE
			std::cout << "Instance Extensions Available:" << std::endl;
			std::vector<ExtensionProperties> Available_Extensions = enumerateInstanceExtensionProperties();
			for (const auto& extension : Available_Extensions) {
				std::cout << extension.extensionName << std::endl;
			}
#endif //DEBUG_MODE
			InstanceCreateInfo createInfo;
			createInfo.pApplicationInfo = &AppInfo;
			std::vector<const char*> enabledExtentions;
			{
				unsigned int extention_count;
				const char ** extentions;
				extentions = glfwGetRequiredInstanceExtensions(&extention_count);
				for (uint32_t i = 0; i < extention_count; i++) {
					enabledExtentions.push_back(extentions[i]);
				}
			}
#ifdef DEBUG_MODE
			enabledExtentions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif //DEBUG_MODE
			createInfo.enabledExtensionCount = enabledExtentions.size();
			createInfo.ppEnabledExtensionNames = enabledExtentions.data();

#ifdef DEBUG_MODE
			createInfo.enabledLayerCount = m_Validation_Layers.size();
			createInfo.ppEnabledLayerNames = m_Validation_Layers.data();
#endif //DEBUG_MODE

			{
				auto err = createInstance(&createInfo, nullptr, &m_Instance);
				Vulkan_Utils::Vulkan_Error(err);
			}

#ifdef DEBUG_MODE
			Vulkan_Utils::Setup_Debug(&m_Instance);
#endif //DEBUG_MODE

			if (glfwCreateWindowSurface((VkInstance)m_Instance, m_Window->getWindowPointer(), nullptr, (VkSurfaceKHR*)&m_Surface) != VK_SUCCESS) {
				throw std::runtime_error("Error: failed to create surface");
			}

			std::vector<PhysicalDevice> physicalDevices;
			physicalDevices = m_Instance.enumeratePhysicalDevices();
			for (PhysicalDevice device : physicalDevices) {
				if (isDeviceSuitable(device)) {
					m_PhysicalDevice = device;
					break;
				}
			}
			if (!m_PhysicalDevice) {
				throw std::runtime_error("Error: Cannot find suitable GPU device");
			}
#ifdef DEBUG_MODE
			std::cout << "Device Extensions Available:" << std::endl;
			std::vector<ExtensionProperties> Device_Available_Extensions = m_PhysicalDevice.enumerateDeviceExtensionProperties();
			for (const auto& extension : Device_Available_Extensions) {
				std::cout << extension.extensionName << std::endl;
			}
#endif //DEBUG_MODE
			std::vector<QueueFamilyProperties> queueFamilies;
			queueFamilies = m_PhysicalDevice.getQueueFamilyProperties();
			findQueueFamilySuitable(queueFamilies);

			std::vector<DeviceQueueCreateInfo> queueCreateInfos;
			std::set<int> uniqueQueueIndex = { m_GraphicsQueueIndex, m_PresentQueueIndex };

			float queuePriorities = 1.0f;
			for (int queueIndex : uniqueQueueIndex) {
				DeviceQueueCreateInfo queueCreateInfo;
				queueCreateInfo.queueFamilyIndex = queueIndex;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriorities;
				queueCreateInfos.push_back(queueCreateInfo);
			}


			PhysicalDeviceFeatures deviceFeatures; // TODO: when features are required

			DeviceCreateInfo deviceInfo;
			deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
			deviceInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
			deviceInfo.pEnabledFeatures = &deviceFeatures;
			deviceInfo.enabledExtensionCount = m_DeviceExtentions.size();
			deviceInfo.ppEnabledExtensionNames = m_DeviceExtentions.data();

#ifdef DEBUG_MODE
			deviceInfo.enabledLayerCount = m_Validation_Layers.size();
			deviceInfo.ppEnabledLayerNames = m_Validation_Layers.data();
#endif //DEBUG_MODE

			m_Device = m_PhysicalDevice.createDevice(deviceInfo);

			m_GraphicsQueue = m_Device.getQueue(m_GraphicsQueueIndex, 0);
			m_PresentQueue = m_Device.getQueue(m_PresentQueueIndex, 0);

			m_Swapchain = CreateSwapchain();
			CreateImageViews();
			CreateRenderPass();
			CreateDescriptorSetLayout();
			CreatePipeline();
			CreateCommandPool();
			CreateDepthResources();
			CreateFrameBuffers();
			CreateTextureImage();
			CreateTextureImageView();
			CreateTextureSampler();
			LoadModel();
			CreateVertexBuffer();
			CreateIndexBuffer();
			CreateUniformBuffer();
			CreateDescriptorPool();
			CreateDescriptorSet();
			CreateCommandBuffers();

			SemaphoreCreateInfo semaphoreInfo;
			m_ImageSemaphore = m_Device.createSemaphore(semaphoreInfo);
			m_RenderSemaphore = m_Device.createSemaphore(semaphoreInfo);
			if (!(m_ImageSemaphore || m_RenderSemaphore)) throw std::runtime_error("Error: Semaphores could not be created");

		}

		void VulkanManager::DestroyManager() {
			m_Device.waitIdle();
			m_Device.freeMemory(m_VertexBufferMemory);
			m_Device.freeMemory(m_IndexBufferMemory);
			m_Device.destroyBuffer(m_VertexBuffer);
			m_Device.destroyBuffer(m_IndexBuffer);
			m_Device.destroySemaphore(m_ImageSemaphore);
			m_Device.destroySemaphore(m_RenderSemaphore);
			for (Framebuffer buffer : m_FrameBuffers) {
				m_Device.destroyFramebuffer(buffer);
			}
			m_Device.destroyCommandPool(m_CommandPool);
			m_Device.destroyPipelineLayout(m_PipelineLayout);
			m_Device.destroyRenderPass(m_RenderPass);
			//m_Device.destroyPipelineCache(m_PipelineCache);
			m_Device.destroyPipeline(m_Pipeline);
			for (ImageView view : m_ImageViews) {
				m_Device.destroyImageView(view);
			}
			m_Device.destroySwapchainKHR(m_Swapchain);
			m_Device.destroy();
			Vulkan_Utils::Free_Debug_Callback(&m_Instance);
			m_Instance.destroySurfaceKHR(m_Surface);
			m_Instance.destroy();
		}

		void VulkanManager::CreatePipeline() {
			std::vector<char> Vertex_Shader, Fragment_Shader;
			try {
				Vertex_Shader = Utils::ImportShader("Shaders/Basic_Vert.spv");
				Fragment_Shader = Utils::ImportShader("Shaders/Basic_Frag.spv");
			}
			catch (std::runtime_error e) {
				std::cout << e.what() << std::endl;
			}
			ShaderModule VertexModule;
			ShaderModule FragmentModule;
			CreateShaderModule(Vertex_Shader, VertexModule);
			CreateShaderModule(Fragment_Shader, FragmentModule);

			PipelineShaderStageCreateInfo vertShaderCreateInfo;
			vertShaderCreateInfo.setStage(ShaderStageFlagBits::eVertex);
			vertShaderCreateInfo.module = VertexModule;
			vertShaderCreateInfo.pName = "main";

			PipelineShaderStageCreateInfo fragShaderCreateInfo;
			fragShaderCreateInfo.setStage(ShaderStageFlagBits::eFragment);
			fragShaderCreateInfo.module = FragmentModule;
			fragShaderCreateInfo.pName = "main";

			PipelineDepthStencilStateCreateInfo depthStencil;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = CompareOp::eLess;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.minDepthBounds = 0.0f;
			depthStencil.maxDepthBounds = 1.0f;
			depthStencil.stencilTestEnable = VK_FALSE;

			PipelineShaderStageCreateInfo shaderStages[] = { vertShaderCreateInfo, fragShaderCreateInfo };

			auto bindingDescription = Vertex::getBindingDescription();
			auto attributeDescription = Vertex::getAttributeDescription();

			PipelineVertexInputStateCreateInfo vertexInputInfo;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.vertexAttributeDescriptionCount = attributeDescription.size();
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

			PipelineInputAssemblyStateCreateInfo inputAssembly;
			inputAssembly.topology = PrimitiveTopology::eTriangleList;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			Viewport viewport;
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)m_SwapchainExtent.width;
			viewport.height = (float)m_SwapchainExtent.height;
			viewport.maxDepth = 1.0f;
			viewport.minDepth = 0.0f;

			Rect2D scissor;
			scissor.extent = m_SwapchainExtent;
			scissor.offset = { 0,0 };

			PipelineViewportStateCreateInfo viewportState;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			PipelineRasterizationStateCreateInfo rasteriser;
			rasteriser.depthClampEnable = VK_FALSE;
			rasteriser.rasterizerDiscardEnable = VK_FALSE;
			rasteriser.polygonMode = PolygonMode::eFill;
			rasteriser.lineWidth = 1.0f;
			rasteriser.cullMode = CullModeFlagBits::eBack;
			rasteriser.frontFace = FrontFace::eCounterClockwise;
			rasteriser.depthBiasEnable = VK_FALSE;

			PipelineMultisampleStateCreateInfo multisampling;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = SampleCountFlagBits::e1;
			multisampling.minSampleShading = 1.0f;
			multisampling.pSampleMask = nullptr;
			multisampling.alphaToCoverageEnable = VK_FALSE;
			multisampling.alphaToOneEnable = VK_FALSE;

			PipelineColorBlendAttachmentState colourBlendAttachment;
			colourBlendAttachment.colorWriteMask = ColorComponentFlagBits::eA | ColorComponentFlagBits::eB | ColorComponentFlagBits::eG | ColorComponentFlagBits::eR;
			colourBlendAttachment.blendEnable = VK_FALSE;

			PipelineColorBlendStateCreateInfo colourBlending;
			colourBlending.logicOpEnable = VK_FALSE;
			colourBlending.attachmentCount = 1;
			colourBlending.pAttachments = &colourBlendAttachment;

			DynamicState dynamicStates[] = { DynamicState::eViewport, DynamicState::eLineWidth };
			PipelineDynamicStateCreateInfo dynamicState;
			dynamicState.dynamicStateCount = 2;
			dynamicState.pDynamicStates = dynamicStates;

			DescriptorSetLayout setLayouts[] = { m_DescriptorSetLayout };

			PipelineLayoutCreateInfo layoutInfo;
			layoutInfo.setLayoutCount = 1;
			layoutInfo.pSetLayouts = setLayouts;
			layoutInfo.pushConstantRangeCount = 0;
			layoutInfo.pPushConstantRanges = nullptr;

			m_PipelineLayout = m_Device.createPipelineLayout(layoutInfo);

			GraphicsPipelineCreateInfo pipelineInfo;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasteriser;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = &depthStencil;
			pipelineInfo.pColorBlendState = &colourBlending;
			pipelineInfo.pDynamicState = nullptr;
			pipelineInfo.layout = m_PipelineLayout;
			pipelineInfo.renderPass = m_RenderPass;
			pipelineInfo.subpass = 0;

			//PipelineCacheCreateInfo cacheInfo;
			//m_PipelineCache = m_Device.createPipelineCache(cacheInfo);

			m_Pipeline = m_Device.createGraphicsPipeline(PipelineCache(), pipelineInfo);

			m_Device.destroyShaderModule(VertexModule);
			m_Device.destroyShaderModule(FragmentModule);
		}

		void VulkanManager::CreateFrameBuffers() {
			m_FrameBuffers.clear();
			for (size_t i = 0; i < m_ImageViews.size(); i++) {
				std::array<ImageView, 2> attachments = { m_ImageViews[i], m_DepthImageView };
				FramebufferCreateInfo framebufferInfo;
				framebufferInfo.renderPass = m_RenderPass;
				framebufferInfo.attachmentCount = attachments.size();
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = m_SwapchainExtent.width;
				framebufferInfo.height = m_SwapchainExtent.height;
				framebufferInfo.layers = 1;
				m_FrameBuffers.push_back(m_Device.createFramebuffer(framebufferInfo));
			}
		}

		void VulkanManager::CreateCommandPool() {
			if (!(m_CommandBuffers.size() > 0)) {
				CommandPoolCreateInfo commandPoolInfo;
				commandPoolInfo.queueFamilyIndex = m_GraphicsQueueIndex;

				m_CommandPool = m_Device.createCommandPool(commandPoolInfo);
			}
			else {
				m_Device.freeCommandBuffers(m_CommandPool, m_CommandBuffers);
			}
		}

		void VulkanManager::CreateCommandBuffers() {

			CommandBufferAllocateInfo allocInfo;
			allocInfo.commandPool = m_CommandPool;
			allocInfo.level = CommandBufferLevel::ePrimary;
			allocInfo.commandBufferCount = m_FrameBuffers.size();

			m_CommandBuffers = m_Device.allocateCommandBuffers(allocInfo);


			for (size_t i = 0; i < m_CommandBuffers.size(); i++) {
				CommandBufferBeginInfo beginInfo;
				beginInfo.flags = CommandBufferUsageFlagBits::eSimultaneousUse;
				beginInfo.pInheritanceInfo = nullptr;

				m_CommandBuffers[i].begin(beginInfo);

				RenderPassBeginInfo renderBeginInfo;
				renderBeginInfo.renderPass = m_RenderPass;
				renderBeginInfo.framebuffer = m_FrameBuffers[i];
				renderBeginInfo.renderArea.offset = { 0,0 };
				renderBeginInfo.renderArea.extent = m_SwapchainExtent;

				std::array<ClearValue, 2> clearValues = {};
				clearValues[0].color.setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });
				clearValues[1].depthStencil.setDepth(1.0f);

				renderBeginInfo.clearValueCount = clearValues.size();
				renderBeginInfo.pClearValues = clearValues.data();

				m_CommandBuffers[i].beginRenderPass(renderBeginInfo, SubpassContents::eInline);
				m_CommandBuffers[i].bindPipeline(PipelineBindPoint::eGraphics, m_Pipeline);
				Buffer vertexBuffers[] = { m_VertexBuffer };
				DeviceSize offsets[] = { 0 };
				m_CommandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
				m_CommandBuffers[i].bindIndexBuffer(m_IndexBuffer, 0, IndexType::eUint32);
				m_CommandBuffers[i].bindDescriptorSets(PipelineBindPoint::eGraphics, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
				m_CommandBuffers[i].drawIndexed(indices.size(), 1, 0, 0, 0);
				m_CommandBuffers[i].endRenderPass();
				m_CommandBuffers[i].end();
			}
		}

		void VulkanManager::CreateImageViews() {
			m_SwapchainImages = m_Device.getSwapchainImagesKHR(m_Swapchain);
			m_ImageCount = m_SwapchainImages.size();

			m_ImageViews.clear();
			m_ImageViews.resize(m_ImageCount);

			for (uint32_t i = 0; i < m_SwapchainImages.size(); i++) {
				CreateImageView(m_SwapchainImages[i], m_SwapchainImageFormat, ImageAspectFlagBits::eColor, m_ImageViews[i]);
			}
		}

		void VulkanManager::CreateVertexBuffer() {
			DeviceSize size = sizeof(vertices[0]) * vertices.size();

			Buffer StagingBuffer;
			DeviceMemory StagingBufferMemory;

			CreateBuffer(size, BufferUsageFlagBits::eTransferSrc, MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent, StagingBuffer, StagingBufferMemory);

			void* data;
			data = m_Device.mapMemory(StagingBufferMemory, 0, size);
			memcpy(data, vertices.data(), (size_t)size);
			m_Device.unmapMemory(StagingBufferMemory);

			CreateBuffer(size, BufferUsageFlagBits::eTransferDst | BufferUsageFlagBits::eVertexBuffer, MemoryPropertyFlagBits::eDeviceLocal, m_VertexBuffer, m_VertexBufferMemory);

			CopyBuffer(StagingBuffer, m_VertexBuffer, size);

			m_Device.freeMemory(StagingBufferMemory);
			m_Device.destroyBuffer(StagingBuffer);
		}

		void VulkanManager::CreateIndexBuffer() {
			DeviceSize size = sizeof(indices[0]) * indices.size();

			Buffer StagingBuffer;
			DeviceMemory StagingBufferMemory;

			CreateBuffer(size, BufferUsageFlagBits::eTransferSrc, MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent, StagingBuffer, StagingBufferMemory);

			void* data;
			data = m_Device.mapMemory(StagingBufferMemory, 0, size);
			memcpy(data, indices.data(), (size_t)size);
			m_Device.unmapMemory(StagingBufferMemory);

			CreateBuffer(size, BufferUsageFlagBits::eTransferDst | BufferUsageFlagBits::eIndexBuffer, MemoryPropertyFlagBits::eDeviceLocal, m_IndexBuffer, m_IndexBufferMemory);

			CopyBuffer(StagingBuffer, m_IndexBuffer, size);

			m_Device.freeMemory(StagingBufferMemory);
			m_Device.destroyBuffer(StagingBuffer);
		}

		void VulkanManager::CreateUniformBuffer() {
			DeviceSize bufferSize = sizeof(UniformBufferObject);

			CreateBuffer(bufferSize, BufferUsageFlagBits::eTransferSrc, MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent, m_UniformStagingBuffer, m_UniformStagingBufferMemory);
			CreateBuffer(bufferSize, BufferUsageFlagBits::eTransferDst | BufferUsageFlagBits::eUniformBuffer, MemoryPropertyFlagBits::eDeviceLocal, m_UniformBuffer, m_UniformBufferMemory);

		}

		void VulkanManager::CreateBuffer(DeviceSize size, BufferUsageFlags usageFlags, MemoryPropertyFlags properties, Buffer &buffer, DeviceMemory &bufferMemory) {
			BufferCreateInfo bufferInfo;
			bufferInfo.size = size;
			bufferInfo.usage = usageFlags;
			bufferInfo.sharingMode = SharingMode::eExclusive;
			buffer = m_Device.createBuffer(bufferInfo);

			MemoryRequirements Memory_Req;
			Memory_Req = m_Device.getBufferMemoryRequirements(buffer);

			MemoryAllocateInfo allocInfo;
			allocInfo.allocationSize = Memory_Req.size;
			allocInfo.memoryTypeIndex = findMemoryType(Memory_Req.memoryTypeBits, properties);
			bufferMemory = m_Device.allocateMemory(allocInfo);
			m_Device.bindBufferMemory(buffer, bufferMemory, 0);
		}

		void VulkanManager::CopyBuffer(Buffer src, Buffer dst, DeviceSize size) {
			CommandBuffer buffer = BeginSingleTimeCommand();

			BufferCopy copy;
			copy.srcOffset = 0;
			copy.dstOffset = 0;
			copy.size = size;
			buffer.copyBuffer(src, dst, 1, &copy);

			EndSingleTimeCommand(buffer);
		}

		uint32_t VulkanManager::findMemoryType(uint32_t typeFilter, MemoryPropertyFlags properties) {
			PhysicalDeviceMemoryProperties deviceMem;
			deviceMem = m_PhysicalDevice.getMemoryProperties();
			for (uint32_t i = 0; i < deviceMem.memoryTypeCount; i++) {
				if ((typeFilter & (1 << i)) && ((deviceMem.memoryTypes[i].propertyFlags & properties) == properties)) return i;
			}

		}

		void VulkanManager::CreateRenderPass() {
			AttachmentDescription colourAttachment;
			colourAttachment.format = m_SwapchainImageFormat;
			colourAttachment.samples = SampleCountFlagBits::e1;
			colourAttachment.loadOp = AttachmentLoadOp::eClear;
			colourAttachment.storeOp = AttachmentStoreOp::eStore;
			colourAttachment.stencilLoadOp = AttachmentLoadOp::eDontCare;
			colourAttachment.stencilStoreOp = AttachmentStoreOp::eDontCare;
			colourAttachment.initialLayout = ImageLayout::eUndefined;
			colourAttachment.finalLayout = ImageLayout::ePresentSrcKHR;

			AttachmentDescription DepthAttachment;
			DepthAttachment.format = FindDepthFormat();
			DepthAttachment.samples = SampleCountFlagBits::e1;
			DepthAttachment.loadOp = AttachmentLoadOp::eClear;
			DepthAttachment.storeOp = AttachmentStoreOp::eDontCare;
			DepthAttachment.stencilLoadOp = AttachmentLoadOp::eDontCare;
			DepthAttachment.stencilStoreOp = AttachmentStoreOp::eDontCare;
			DepthAttachment.initialLayout = ImageLayout::eDepthStencilAttachmentOptimal;
			DepthAttachment.finalLayout = ImageLayout::eDepthStencilAttachmentOptimal;

			AttachmentReference depthRef;
			depthRef.attachment = 1;
			depthRef.layout = ImageLayout::eDepthStencilAttachmentOptimal;

			AttachmentReference colourRef;
			colourRef.attachment = 0;
			colourRef.layout = ImageLayout::eColorAttachmentOptimal;

			SubpassDescription subPass;
			subPass.pipelineBindPoint = PipelineBindPoint::eGraphics;
			subPass.colorAttachmentCount = 1;
			subPass.pColorAttachments = &colourRef;
			subPass.pDepthStencilAttachment = &depthRef;

			SubpassDependency dependency;
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = PipelineStageFlagBits::eBottomOfPipe;
			dependency.srcAccessMask = AccessFlagBits::eMemoryRead;
			dependency.dstStageMask = PipelineStageFlagBits::eBottomOfPipe;
			dependency.dstAccessMask = AccessFlagBits::eMemoryRead | AccessFlagBits::eMemoryWrite;

			std::array<AttachmentDescription, 2> attachments = { colourAttachment, DepthAttachment };
			RenderPassCreateInfo createInfo;
			createInfo.attachmentCount = attachments.size();
			createInfo.pAttachments = attachments.data();
			createInfo.subpassCount = 1;
			createInfo.pSubpasses = &subPass;
			createInfo.dependencyCount = 1;
			createInfo.pDependencies = &dependency;

			m_RenderPass = m_Device.createRenderPass(createInfo);
		}

		void VulkanManager::CreateShaderModule(const std::vector<char>& code, ShaderModule& module) {
			ShaderModuleCreateInfo createInfo;
			createInfo.codeSize = code.size();
			createInfo.pCode = (uint32_t*)code.data();
			module = m_Device.createShaderModule(createInfo);
			if (!(module)) throw std::runtime_error("Error: Shader Module cannot be created");

		}


		bool VulkanManager::isDeviceSuitable(PhysicalDevice device) {
			PhysicalDeviceFeatures deviceFeatures = device.getFeatures();
			PhysicalDeviceProperties deviceProperties = device.getProperties();
			if (!((deviceProperties.deviceType == PhysicalDeviceType::eDiscreteGpu) && deviceFeatures.geometryShader)) {
				return false;
			}
			if (!checkDeviceExtensionsSupport(device)) return false;
			m_SwapchainSupportDetails details = querySwapchain(device);
			if (details.formats.empty() || details.presentModes.empty()) return false;
			return true;
		}

		bool VulkanManager::checkDeviceExtensionsSupport(PhysicalDevice device) {
			std::vector<ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();
			std::set<std::string> requiredExtensions(m_DeviceExtentions.begin(), m_DeviceExtentions.end());
			for (const auto& extension : availableExtensions) {
				requiredExtensions.erase(extension.extensionName);
			}
			return requiredExtensions.empty();
		}

		VulkanManager::m_SwapchainSupportDetails VulkanManager::querySwapchain(PhysicalDevice device) {
			m_SwapchainSupportDetails details;
			details.capabilities = device.getSurfaceCapabilitiesKHR(m_Surface);
			details.formats = device.getSurfaceFormatsKHR(m_Surface);
			details.presentModes = device.getSurfacePresentModesKHR(m_Surface);
			return details;
		}

		void VulkanManager::findQueueFamilySuitable(std::vector<QueueFamilyProperties> queueFamilies) {
			for (uint32_t i = 0; i < queueFamilies.size(); i++) {
				if (queueFamilies[i].queueCount > 0 && m_PhysicalDevice.getSurfaceSupportKHR(i, m_Surface)) {
					m_PresentQueueIndex = i;
				}
				if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & QueueFlagBits::eGraphics) {
					m_GraphicsQueueIndex = i;
				}
			}
			if (m_GraphicsQueueIndex > -1 && m_PresentQueueIndex > -1) return;
			throw std::runtime_error("Error: Suitable Queue could not be found");
		}

		SurfaceFormatKHR VulkanManager::chooseSwapchainFormat(const std::vector<SurfaceFormatKHR> availableFormats) {
			if (availableFormats.size() == 1 && availableFormats[0].format == Format::eUndefined) {
				return{ Format::eB8G8R8A8Unorm, ColorSpaceKHR::eSrgbNonlinear };
			}
			for (const auto& f_format : availableFormats) {
				if (f_format.format == Format::eB8G8R8A8Unorm && f_format.colorSpace == ColorSpaceKHR::eSrgbNonlinear) return f_format;
			}
			return availableFormats[0];
		}

		PresentModeKHR VulkanManager::choosePresentMode(const std::vector<PresentModeKHR> availableModes) {
			for (const auto& f_mode : availableModes) {
				if (f_mode == PresentModeKHR::eMailbox) return f_mode;
			}
			return PresentModeKHR::eFifo;
		}

		Extent2D VulkanManager::chooseSwapchainExtent(const SurfaceCapabilitiesKHR capabilities) {
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;
			Extent2D actualExtent = { (uint32_t)m_Window->getWidth(), (uint32_t)m_Window->getHeight() };
			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
			return actualExtent;
		}

		SwapchainKHR VulkanManager::CreateSwapchain(SwapchainKHR &oldSwapchain) {
			m_SwapchainSupportDetails SwapchainDetails = querySwapchain(m_PhysicalDevice);
			SurfaceFormatKHR SwapchainFormat = chooseSwapchainFormat(SwapchainDetails.formats);
			PresentModeKHR SwapchainPresentMode = choosePresentMode(SwapchainDetails.presentModes);
			m_SwapchainExtent = chooseSwapchainExtent(SwapchainDetails.capabilities);
			uint32_t ImageCount = SwapchainDetails.capabilities.minImageCount + 1;
			m_SwapchainImageFormat = SwapchainFormat.format;

			if (SwapchainDetails.capabilities.maxImageCount > 0 && ImageCount > SwapchainDetails.capabilities.maxImageCount) {
				ImageCount = SwapchainDetails.capabilities.maxImageCount;
			}

			SwapchainCreateInfoKHR createInfo;
			createInfo.surface = m_Surface;
			createInfo.minImageCount = ImageCount;
			createInfo.imageFormat = SwapchainFormat.format;
			createInfo.imageColorSpace = SwapchainFormat.colorSpace;
			createInfo.imageExtent = m_SwapchainExtent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = ImageUsageFlagBits::eColorAttachment;
			if (m_GraphicsQueueIndex != m_PresentQueueIndex) {
				uint32_t queueIndicies[] = { (uint32_t)m_GraphicsQueueIndex, (uint32_t)m_PresentQueueIndex };
				createInfo.imageSharingMode = SharingMode::eConcurrent;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueIndicies;
			}
			else {
				createInfo.imageSharingMode = SharingMode::eExclusive;
				createInfo.queueFamilyIndexCount = 0;
				createInfo.pQueueFamilyIndices = nullptr;
			}
			createInfo.preTransform = SwapchainDetails.capabilities.currentTransform;
			createInfo.compositeAlpha = CompositeAlphaFlagBitsKHR::eOpaque;
			createInfo.presentMode = SwapchainPresentMode;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = oldSwapchain;

			return m_Device.createSwapchainKHR(createInfo);
		}

		void VulkanManager::DrawFrame() {
			uint32_t ImageIndex;
			ResultValue<uint32_t> value = m_Device.acquireNextImageKHR(m_Swapchain, std::numeric_limits<uint64_t>::max(), m_ImageSemaphore, Fence());
			if (value.result == Result::eErrorOutOfDateKHR) {
				RecreateSwapchain();
				return;
			}
			ImageIndex = value.value;

			SubmitInfo submitInfo;
			PipelineStageFlags waitStages[] = { PipelineStageFlagBits::eColorAttachmentOutput };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &m_ImageSemaphore;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_CommandBuffers[ImageIndex];
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &m_RenderSemaphore;

			m_GraphicsQueue.submit(submitInfo, Fence());

			PresentInfoKHR presentInfo;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &m_RenderSemaphore;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &m_Swapchain;
			presentInfo.pImageIndices = &ImageIndex;

			if (m_PresentQueue.presentKHR(presentInfo) == Result::eErrorOutOfDateKHR) {
				RecreateSwapchain();
			}
		}

		void VulkanManager::RecreateSwapchain() {
			m_Device.waitIdle();

			m_Swapchain = CreateSwapchain(m_Swapchain);
			CreateImageViews();
			CreateRenderPass();
			CreatePipeline();
			CreateDepthResources();
			CreateFrameBuffers();
			CreateCommandBuffers();
		}

		void VulkanManager::CreateDescriptorSetLayout() {
			DescriptorSetLayoutBinding UBOlayoutBinding;
			UBOlayoutBinding.binding = 0;
			UBOlayoutBinding.descriptorType = DescriptorType::eUniformBuffer;
			UBOlayoutBinding.descriptorCount = 1;
			UBOlayoutBinding.stageFlags = ShaderStageFlagBits::eVertex;

			DescriptorSetLayoutBinding SamplerLayoutBinding;
			SamplerLayoutBinding.binding = 1;
			SamplerLayoutBinding.descriptorCount = 1;
			SamplerLayoutBinding.descriptorType = DescriptorType::eCombinedImageSampler;
			SamplerLayoutBinding.pImmutableSamplers = nullptr;
			SamplerLayoutBinding.stageFlags = ShaderStageFlagBits::eFragment;

			std::array<DescriptorSetLayoutBinding, 2> bindings = { UBOlayoutBinding, SamplerLayoutBinding };
			DescriptorSetLayoutCreateInfo info;
			info.bindingCount = bindings.size();
			info.pBindings = bindings.data();

			m_DescriptorSetLayout = m_Device.createDescriptorSetLayout(info);
		}

		void VulkanManager::UpdateUniformBuffers() {
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto now = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count() / 1000000.0f;

			UniformBufferObject UBO = {};
			UBO.model = rotate(mat4(), time*radians(90.0f), vec3(0, 0, 1.0f));
			UBO.view = lookAt(vec3(2.0f, 2.0f, 2.0f), vec3(0, 0, 0), vec3(0, 0, 1.0f));
			UBO.proj = perspective(radians(45.0f), m_SwapchainExtent.width / (float)m_SwapchainExtent.height, 0.1f, 10.0f);
			UBO.proj[1][1] *= -1;

			void* data;
			data = m_Device.mapMemory(m_UniformStagingBufferMemory, 0, sizeof(UBO));
			memcpy(data, &UBO, sizeof(UBO));
			m_Device.unmapMemory(m_UniformStagingBufferMemory);

			CopyBuffer(m_UniformStagingBuffer, m_UniformBuffer, sizeof(UBO));
		}

		void VulkanManager::CreateDescriptorPool() {
			DescriptorPoolSize poolSize;
			poolSize.type = DescriptorType::eUniformBuffer;
			poolSize.descriptorCount = 1;
			DescriptorPoolSize poolSize2;
			poolSize2.type = DescriptorType::eCombinedImageSampler;
			poolSize2.descriptorCount = 1;
			std::array<DescriptorPoolSize, 2> poolSizes = { poolSize, poolSize2 };

			DescriptorPoolCreateInfo poolInfo;
			poolInfo.poolSizeCount = poolSizes.size();
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = 1;

			m_DescriptorPool = m_Device.createDescriptorPool(poolInfo);
		}

		void VulkanManager::CreateDescriptorSet() {
			DescriptorSetLayout layouts[] = { m_DescriptorSetLayout };
			DescriptorSetAllocateInfo allocInfo;
			allocInfo.descriptorPool = m_DescriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = layouts;

			m_DescriptorSet = m_Device.allocateDescriptorSets(allocInfo)[0];

			DescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = m_UniformBuffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			DescriptorImageInfo imageInfo;
			imageInfo.imageLayout = ImageLayout::eShaderReadOnlyOptimal;
			imageInfo.imageView = m_TextureImageView;
			imageInfo.sampler = m_TextureSampler;

			std::array<WriteDescriptorSet, 2> descriptorWrites = {};

			descriptorWrites[0].dstSet = m_DescriptorSet;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = DescriptorType::eUniformBuffer;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].dstSet = m_DescriptorSet;
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = DescriptorType::eCombinedImageSampler;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;
			
			m_Device.updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

		}

		void VulkanManager::CreateTextureImage() {
			int t_Width, t_Height, t_Channels;
			stbi_uc* pixels = stbi_load("Textures/Chalet.jpg", &t_Width, &t_Height, &t_Channels, STBI_rgb_alpha);
			DeviceSize imageSize = t_Width * t_Height * 4;
			if (!pixels) {
				throw std::runtime_error("Error: failed to load Image");
			}

			CreateImage(t_Width, t_Height, Format::eR8G8B8A8Unorm, ImageTiling::eLinear, ImageUsageFlagBits::eTransferSrc, MemoryPropertyFlagBits::eHostCoherent | MemoryPropertyFlagBits::eHostVisible, m_StagingImage, m_StagingImageMemory);

			void* data;
			data = m_Device.mapMemory(m_StagingImageMemory, 0, imageSize);
			memcpy(data, pixels, (size_t)imageSize);
			m_Device.unmapMemory(m_StagingImageMemory);

			stbi_image_free(pixels);

			CreateImage(t_Width, t_Height, Format::eR8G8B8A8Unorm, ImageTiling::eOptimal, ImageUsageFlagBits::eTransferDst | ImageUsageFlagBits::eSampled, MemoryPropertyFlagBits::eDeviceLocal, m_TextureImage, m_TextureImageMemory);

			TransitionImageLayout(m_StagingImage, Format::eR8G8B8A8Unorm, ImageLayout::ePreinitialized, ImageLayout::eTransferSrcOptimal);
			TransitionImageLayout(m_TextureImage, Format::eR8G8B8A8Unorm, ImageLayout::ePreinitialized, ImageLayout::eTransferDstOptimal);
			CopyImage(m_StagingImage, m_TextureImage, t_Width, t_Height);
			TransitionImageLayout(m_TextureImage, Format::eR8G8B8A8Unorm, ImageLayout::eTransferDstOptimal, ImageLayout::eShaderReadOnlyOptimal);


		}

		void VulkanManager::CreateImage(uint32_t width, uint32_t height, Format format, ImageTiling tiling, ImageUsageFlags usage, MemoryPropertyFlags properties, Image &image, DeviceMemory &imageMemory) {
			ImageCreateInfo imageInfo;
			imageInfo.imageType = ImageType::e2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = tiling;
			imageInfo.initialLayout = ImageLayout::ePreinitialized;
			imageInfo.usage = usage;
			imageInfo.sharingMode = SharingMode::eExclusive;
			imageInfo.samples = SampleCountFlagBits::e1;

			image = m_Device.createImage(imageInfo);

			MemoryRequirements memRequirements;
			memRequirements = m_Device.getImageMemoryRequirements(image);

			MemoryAllocateInfo allocInfo;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

			imageMemory = m_Device.allocateMemory(allocInfo);
			
		m_Device.bindImageMemory(image, imageMemory, 0);
	}

	void VulkanManager::TransitionImageLayout(Image image, Format format, ImageLayout oldLayout, ImageLayout newLayout) {
		CommandBuffer buffer = BeginSingleTimeCommand();
			
		ImageMemoryBarrier barrier;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		if (oldLayout == ImageLayout::ePreinitialized && newLayout == ImageLayout::eTransferSrcOptimal) {
			barrier.srcAccessMask = AccessFlagBits::eHostWrite;
			barrier.dstAccessMask = AccessFlagBits::eTransferRead;
		}
		else if (oldLayout == ImageLayout::ePreinitialized && newLayout == ImageLayout::eTransferDstOptimal) {
			barrier.srcAccessMask = AccessFlagBits::eHostWrite;
			barrier.dstAccessMask = AccessFlagBits::eTransferWrite;
		}
		else if (oldLayout == ImageLayout::eTransferDstOptimal && newLayout == ImageLayout::eShaderReadOnlyOptimal) {
			barrier.srcAccessMask = AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = AccessFlagBits::eShaderRead;
		}
		else if (oldLayout == ImageLayout::eUndefined && newLayout == ImageLayout::eDepthStencilAttachmentOptimal) {
			barrier.srcAccessMask = (AccessFlagBits)0;
			barrier.dstAccessMask = AccessFlagBits::eDepthStencilAttachmentRead | AccessFlagBits::eDepthStencilAttachmentWrite;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}
			
		if (newLayout == ImageLayout::eDepthStencilAttachmentOptimal) {
			barrier.subresourceRange.aspectMask = ImageAspectFlagBits::eDepth;

			if (HasStencilComponent(format)) {
				barrier.subresourceRange.aspectMask |= ImageAspectFlagBits::eStencil;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = ImageAspectFlagBits::eColor;
		}

		buffer.pipelineBarrier(PipelineStageFlagBits::eTopOfPipe, PipelineStageFlagBits::eTopOfPipe, (DependencyFlagBits)0, 0, nullptr, 0, nullptr, 1, &barrier);
			
		EndSingleTimeCommand(buffer);
	}

	void VulkanManager::CopyImage(Image srcImage, Image dstImage, uint32_t width, uint32_t height) {
		CommandBuffer buffer = BeginSingleTimeCommand();
			
		ImageSubresourceLayers subResource;
		subResource.aspectMask = ImageAspectFlagBits::eColor;
		subResource.baseArrayLayer = 0;
		subResource.mipLevel = 0;
		subResource.layerCount = 1;
		
		ImageCopy region;
		region.srcSubresource = subResource;
		region.dstSubresource = subResource;
		region.srcOffset = { 0, 0, 0 };
		region.dstOffset = { 0, 0, 0 };
		region.extent.width = width;
		region.extent.height = height;
		region.extent.depth = 1;

		buffer.copyImage(srcImage, ImageLayout::eTransferSrcOptimal, dstImage, ImageLayout::eTransferDstOptimal, 1, &region);

		EndSingleTimeCommand(buffer);
	}

	CommandBuffer VulkanManager::BeginSingleTimeCommand() {
		CommandBufferAllocateInfo info;
		info.level = CommandBufferLevel::ePrimary;
		info.commandPool = m_CommandPool;
		info.commandBufferCount = 1;

		CommandBuffer buffer;
		buffer = m_Device.allocateCommandBuffers(info)[0];

		CommandBufferBeginInfo beginInfo;
		beginInfo.flags = CommandBufferUsageFlagBits::eOneTimeSubmit;

		buffer.begin(beginInfo);
		return buffer;
	}

	void VulkanManager::EndSingleTimeCommand(CommandBuffer buffer) {
		buffer.end();

		SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &buffer;

		m_GraphicsQueue.submit(submitInfo, Fence());
		m_GraphicsQueue.waitIdle();

		m_Device.freeCommandBuffers(m_CommandPool, buffer);
	}

	void VulkanManager::CreateImageView(Image image, Format format, ImageAspectFlags aspectFlags, ImageView &view) {
		ImageViewCreateInfo createInfo;
		createInfo.image = image;
		createInfo.viewType = ImageViewType::e2D;
		createInfo.format = format;
		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.components.a = ComponentSwizzle::eIdentity;
		createInfo.components.g = ComponentSwizzle::eIdentity;
		createInfo.components.b = ComponentSwizzle::eIdentity;
		createInfo.components.r = ComponentSwizzle::eIdentity;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		view = m_Device.createImageView(createInfo);
	}

	void VulkanManager::CreateTextureImageView() {
		CreateImageView(m_TextureImage, Format::eR8G8B8A8Unorm, ImageAspectFlagBits::eColor, m_TextureImageView);
	}

	void VulkanManager::CreateTextureSampler() {
		SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = Filter::eLinear;
		samplerInfo.minFilter = Filter::eLinear;
		samplerInfo.addressModeU = SamplerAddressMode::eRepeat;
		samplerInfo.addressModeV = SamplerAddressMode::eRepeat;
		samplerInfo.addressModeW = SamplerAddressMode::eRepeat;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = BorderColor::eIntOpaqueBlack;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = CompareOp::eAlways;
		samplerInfo.mipmapMode = SamplerMipmapMode::eLinear;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		m_TextureSampler = m_Device.createSampler(samplerInfo);
	}

	void VulkanManager::CreateDepthResources() {
		Format format = FindDepthFormat();
		CreateImage(m_SwapchainExtent.width, m_SwapchainExtent.height, format, ImageTiling::eOptimal, ImageUsageFlagBits::eDepthStencilAttachment, MemoryPropertyFlagBits::eDeviceLocal, m_DepthImage, m_DepthImageMemory);
		CreateImageView(m_DepthImage, format, ImageAspectFlagBits::eDepth, m_DepthImageView);
		TransitionImageLayout(m_DepthImage, format, ImageLayout::eUndefined, ImageLayout::eDepthStencilAttachmentOptimal);
	}

	bool VulkanManager::HasStencilComponent(Format format) {
		return format == Format::eD32SfloatS8Uint || format == Format::eD24UnormS8Uint;
	}

	Format VulkanManager::FindDepthFormat() {
		return FindSupportedFormat({ Format::eD32Sfloat, Format::eD32SfloatS8Uint, Format::eD24UnormS8Uint }, ImageTiling::eOptimal, FormatFeatureFlagBits::eDepthStencilAttachment);
	}

	Format VulkanManager::FindSupportedFormat(const std::vector<Format>& formats, ImageTiling tiling, FormatFeatureFlags features) {
		for (Format format : formats) {
			FormatProperties props;
			props = m_PhysicalDevice.getFormatProperties(format);

			if (tiling == ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if(tiling == ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		throw std::runtime_error("Error: Could not find a suitable format");
	}

	void VulkanManager::LoadModel() {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "Models/chalet.obj")) {
			throw std::runtime_error(err);
		}

		std::unordered_map<Vertex, int> uniqueVertices = {};

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};				

				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				vertex.texture = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1 - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.colour = { 1.0f, 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = vertices.size();
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}
}}