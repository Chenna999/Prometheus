#include "Renderer.h"

namespace Prometheus {namespace Graphics {

	RenderManager::RenderManager(VulkanManager* manager){
		vulkan_manager = manager;
	}
}}