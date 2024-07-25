#include <stdexcept>

#include "Surface.h"

#include "VulkanInstance.h"

Surface::Surface()
	: m_VkSurface{ VK_NULL_HANDLE }
{
}

void Surface::Initialize(VkInstance instance, GLFWwindow* window)
{
	if (glfwCreateWindowSurface(instance, window, VK_NULL_HANDLE, &m_VkSurface) != VK_SUCCESS)
	{
		throw std::runtime_error{ "failed to create window surface!" };
	}
}

void Surface::Destroy(VkInstance instance)
{
	if (m_VkSurface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(instance, m_VkSurface, VK_NULL_HANDLE);
		m_VkSurface = VK_NULL_HANDLE;
	}
}

const VkSurfaceKHR& Surface::GetVkSurface() const
{
	return m_VkSurface;
}