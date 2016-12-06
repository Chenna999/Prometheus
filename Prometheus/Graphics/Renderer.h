#pragma once
#include "Vulkan_Manager.h"

namespace Prometheus{ namespace Graphics {

	class RenderManager {
	private:
		VulkanManager* vulkan_manager;
	public:
		RenderManager(VulkanManager* manager);
	};

}}