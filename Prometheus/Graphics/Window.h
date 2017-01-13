#pragma once
#include <GLFW\glfw3.h>
#include <vector>
#include <iostream>
#include "../Utils/MessageBus.h"

#define M_APP_VERSION VK_MAKE_VERSION(1, 0, 0)
#define M_ENGINE_VERSION VK_MAKE_VERSION(1, 0, 0)
#define M_ENGINE_NAME "Prometheus"

namespace Prometheus { namespace Graphics {
	class Window {
	private:
		int m_Width = 800, m_Height = 600;
		char* m_Name = "Vulkan";
		GLFWwindow* m_Window;
		GLFWmonitor* m_PrimaryMonitor;
		bool m_Running;
		bool m_VSync = false;
	public:
		Window();
		~Window();

		void Update();

		bool isRunning();
		GLFWwindow* getWindowPointer();
		const char* getName();
		int getWidth();
		int getHeight();
		bool getVSync();
		void updateWidth(int newWidth);
		void updateHeight(int newHeight);
		void updateName(const char* newName);
	};
}}
