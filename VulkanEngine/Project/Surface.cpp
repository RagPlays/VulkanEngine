#include <stdexcept>

#include "Surface.h"

#include "VulkanInstance.h"

void Surface::Initialize(VkInstance instance, GLFWwindow* window)
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &m_VkSurface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

void Surface::Destroy(VkInstance instance)
{
	vkDestroySurfaceKHR(instance, m_VkSurface, nullptr);
}

const VkSurfaceKHR& Surface::GetVkSurface() const
{
	return m_VkSurface;
}