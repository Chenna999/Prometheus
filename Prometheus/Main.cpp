#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include <iostream>
#include <thread>
#include "Graphics\Window.h"
#include "Graphics\Vulkan_Manager.h"

using namespace Prometheus;
using namespace Graphics;

struct UserPointer {
	Window* window;
	VulkanManager* manager;
};

static void onWindowResize(GLFWwindow* window, int width, int height);

int main(void) {
	if (glfwInit() != GLFW_TRUE) return -1;
	Window w;
	VulkanManager V_M(&w);
	UserPointer userPointer = { &w, &V_M };
	glfwSetWindowUserPointer(w.getWindowPointer(), &userPointer);
	glfwSetWindowSizeCallback(w.getWindowPointer(), onWindowResize);

	while (w.isRunning()) {
		glfwPollEvents();
		V_M.DrawFrame();
		V_M.UpdateUniformBuffers();
		w.Update();
	}
	
	V_M.DestroyManager();
	glfwTerminate();
	system("PAUSE");
	return 0;
}

static void onWindowResize(GLFWwindow* window, int width, int height) {
	if (width == 0 || height == 0) return;

	UserPointer* userPointer = reinterpret_cast<UserPointer*> (glfwGetWindowUserPointer(window));
	userPointer->window->updateWidth(width);
	userPointer->window->updateHeight(height);
	userPointer->manager->RecreateSwapchain();
}