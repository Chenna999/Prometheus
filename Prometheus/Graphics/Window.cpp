#include "Window.h"

namespace Prometheus { namespace Graphics {

	Window::Window() {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

		m_PrimaryMonitor = glfwGetPrimaryMonitor();

		m_Window = glfwCreateWindow(m_Width, m_Height, m_Name, nullptr, nullptr);

		const GLFWvidmode* mode = glfwGetVideoMode(m_PrimaryMonitor);
		glfwSetWindowPos(m_Window, (mode->width - m_Width) / 2, (mode->height - m_Height) / 2);
		m_Running = true;
	}

	Window::~Window() {

	}

	GLFWwindow* Window::getWindowPointer() {
		return m_Window;
	}

	bool Window::isRunning() {
		for (MessageBus::Message msg : MessageBus::getStack()) {
			if (msg.type == MessageBus::MessageType::WINDOW_SHOULD_CLOSE) return false;
		}
		return true;
	}

	const char* Window::getName() {
		return m_Name;
	}

	int Window::getWidth() {
		return m_Width;
	}

	int Window::getHeight() {
		return m_Height;
	}

	void Window::updateWidth(int newWidth) {
		m_Width = newWidth;
	}

	void Window::updateHeight(int newHeight) {
		m_Height = newHeight;
	}

	void Window::updateName(const char* newName) {
		glfwSetWindowTitle(m_Window, newName);
		m_Name = (char*) newName;
	}

	void Window::Update() {
		if (glfwWindowShouldClose(m_Window)) {
			MessageBus::Message msg;
			msg.type = MessageBus::MessageType::WINDOW_SHOULD_CLOSE;
			MessageBus::pushMessage(msg);
			std::cout << "Message_Pushed" << std::endl;
		}
	}
}}