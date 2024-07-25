#include <iostream>

#include "Window.h"

const std::vector<std::pair<int, int>> Window::s_WindowHints
{
    std::make_pair(GLFW_CLIENT_API, GLFW_NO_API), // make no OpenGL context
    std::make_pair(GLFW_RESIZABLE, GLFW_FALSE) // make unresizable (resizing supported!!)
};

Window::Window()
	: m_pWindow{}
	, m_FramebufferResized{ true }
{
}

void Window::Initialize()
{
	// initializes the GLFW library
	glfwInit();

	// Window Hints
	std::cout << "WindowHints:\n";
	for (const auto& hint : s_WindowHints)
	{
		std::cout << "Hint: " << hint.first << ", Value: " << hint.second << "\n";
		glfwWindowHint(hint.first, hint.second);
	}

	// Window Creating
	m_pWindow = glfwCreateWindow(g_WindowWidth, g_WindowHeight, "VULKAN_ENGINE", nullptr, nullptr);

	// CallBack
	glfwSetWindowUserPointer(m_pWindow, this);
	glfwSetFramebufferSizeCallback(m_pWindow, FramebufferResizeCallback);
}

void Window::Destroy()
{
	if (m_pWindow)
	{
		glfwDestroyWindow(m_pWindow);
		m_pWindow = nullptr;
	}
	glfwTerminate();
}

bool Window::WindowShouldClose() const
{
	return glfwWindowShouldClose(m_pWindow);
}

GLFWwindow* Window::GetWindow() const
{
	return m_pWindow;
}

void Window::GetFramebufferSize(int& width, int& height) const
{
	glfwGetFramebufferSize(m_pWindow, &width, &height);
}

void Window::PollEvents() const
{
	glfwPollEvents();
}

void Window::WaitEvents() const
{
	glfwWaitEvents();
}

float Window::GetAspectRatio() const
{
	return static_cast<float>(g_WindowWidth) / static_cast<float>(g_WindowHeight);
}

bool Window::GetFramebufferResized() const
{
	return m_FramebufferResized;
}

void Window::SetFramebufferResized(bool resized)
{
	m_FramebufferResized = resized;
}

void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app{ reinterpret_cast<Window*>(glfwGetWindowUserPointer(window)) };
	app->m_FramebufferResized = true;
}