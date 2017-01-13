#pragma once

#define GLM_FORCE_RADIANS

#include <vulkan\vulkan.hpp>
#include <iostream>
#include <set>
#include <array>
#include <chrono>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <stb_image.h>
#include <tiny_obj_loader.h>
#include <unordered_map>

#include "Window.h"
#include "../Utils/Utils.h"
#include "Vulkan_Objects\VulkanSwapchain.h"

using namespace vk;

namespace Prometheus { namespace Graphics {

	using namespace glm;

	class VulkanManager {
	private:
		Instance m_Instance;
		PhysicalDevice m_PhysicalDevice;
		Device m_Device;
		SurfaceKHR m_Surface;
		Window* m_Window;
		Queue m_GraphicsQueue, m_PresentQueue;
		DescriptorSetLayout m_DescriptorSetLayout;
		PipelineLayout m_PipelineLayout;
		RenderPass m_RenderPass;
		Pipeline m_Pipeline;
		std::vector<Framebuffer> m_FrameBuffers;
		CommandPool m_CommandPool;
		std::vector<CommandBuffer> m_CommandBuffers;
		Buffer m_VertexBuffer;
		DeviceMemory m_VertexBufferMemory;
		Buffer m_IndexBuffer;
		DeviceMemory m_IndexBufferMemory;

		VulkanSwapchain *m_Swapchain;

		Buffer m_UniformStagingBuffer, m_UniformBuffer;
		DeviceMemory m_UniformStagingBufferMemory, m_UniformBufferMemory;
		DescriptorPool m_DescriptorPool;
		DescriptorSet m_DescriptorSet;

		Image m_StagingImage;
		DeviceMemory m_StagingImageMemory;
		Image m_TextureImage;
		DeviceMemory m_TextureImageMemory;
		ImageView m_TextureImageView;
		Sampler m_TextureSampler;

		Image m_DepthImage;
		DeviceMemory m_DepthImageMemory;
		ImageView m_DepthImageView;

		Semaphore m_ImageSemaphore, m_RenderSemaphore;

		
		const std::vector<const char*> m_DeviceExtentions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		const std::vector<const char*> m_Validation_Layers = {
			"VK_LAYER_LUNARG_standard_validation"
		};

		struct UniformBufferObject {
			mat4 model;
			mat4 view;
			mat4 proj;
		};

	public:
		struct Vertex {
			vec3 position;
			vec4 colour;
			vec2 texture;

			static VertexInputBindingDescription getBindingDescription() {
				VertexInputBindingDescription m_BindingDescription;
				m_BindingDescription.binding = 0;
				m_BindingDescription.stride = sizeof(Vertex);
				m_BindingDescription.inputRate = VertexInputRate::eVertex;
				return m_BindingDescription;
			}

			static std::array<VertexInputAttributeDescription, 3> getAttributeDescription() {
				std::array<VertexInputAttributeDescription, 3> attributeDescription = {};
				attributeDescription[0].binding = 0;
				attributeDescription[0].location = 0;
				attributeDescription[0].format = Format::eR32G32B32Sfloat;
				attributeDescription[0].offset = offsetof(Vertex, position);

				attributeDescription[1].binding = 0;
				attributeDescription[1].location = 1;
				attributeDescription[1].format = Format::eR32G32B32A32Sfloat;
				attributeDescription[1].offset = offsetof(Vertex, colour);

				attributeDescription[2].binding = 0;
				attributeDescription[2].location = 2;
				attributeDescription[2].format = Format::eR32G32Sfloat;
				attributeDescription[2].offset = offsetof(Vertex, texture);

				return attributeDescription;
			}

			bool operator==(const Vertex& other) const {
				return position == other.position && colour == other.colour && texture == other.texture;
			}

			
		};

	private:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		
	public:
		VulkanManager(Window* window);
		~VulkanManager();
		void DestroyManager();
		void DrawFrame();
		void RecreateSwapchain();
		void UpdateUniformBuffers();
	private:
		bool isDeviceSuitable(PhysicalDevice device);
		VulkanSwapchain::QueueIndecieSet findQueueFamilySuitable(std::vector<QueueFamilyProperties> queueFamilies);
		uint32_t findMemoryType(uint32_t typeFilter, MemoryPropertyFlags properties);
		bool checkDeviceExtensionsSupport(PhysicalDevice device);
		void InitVulkan();
		void CreateShaderModule(const std::vector<char>& code, ShaderModule& module);

		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreatePipeline();
		void CreateCommandPool();
		void CreateDepthResources();
		void CreateFrameBuffers();
		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();
		void LoadModel();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateUniformBuffer();
		void CreateDescriptorPool();
		void CreateDescriptorSet();
		void CreateCommandBuffers();

		void CreateBuffer(DeviceSize size, BufferUsageFlags usageFlags, MemoryPropertyFlags properties, Buffer &buffer, DeviceMemory &bufferMemory);
		void CopyBuffer(Buffer src, Buffer dst, DeviceSize size);
		void CreateImage(uint32_t width, uint32_t height, Format format, ImageTiling tiling, ImageUsageFlags usage, MemoryPropertyFlags properties, Image &image, DeviceMemory &imageMemory);
		void TransitionImageLayout(Image image, Format format, ImageLayout oldLayout, ImageLayout newLayout);
		void CopyImage(Image srcImage, Image dstImage, uint32_t width, uint32_t height);
		Format FindSupportedFormat(const std::vector<Format>& formats, ImageTiling tiling, FormatFeatureFlags features);
		Format FindDepthFormat();
		bool HasStencilComponent(Format format);
		CommandBuffer BeginSingleTimeCommand();
		void EndSingleTimeCommand(CommandBuffer buffer);
	};

	
}}

template<> struct std::hash<Prometheus::Graphics::VulkanManager::Vertex> {
	size_t operator()(Prometheus::Graphics::VulkanManager::Vertex const& vertex) const {
		return ((hash<glm::vec3>()(vertex.position) ^
			(hash<glm::vec3>()(vertex.colour) << 1)) >> 1) ^
			(hash<glm::vec2>()(vertex.texture) << 1);
	}
};

