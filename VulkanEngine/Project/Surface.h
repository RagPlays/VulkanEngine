#ifndef SURFACE_H
#define SURFACE_H

#include <vulkan/vulkan.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

class VulkanInstance;

class Surface final
{
public:

	Surface() = default;
	~Surface() = default;

	void Initialize(VkInstance instance, GLFWwindow* window);
	void Destroy(VkInstance instance);

	const VkSurfaceKHR& GetVkSurface() const;

private:

	VkSurfaceKHR m_VkSurface;

};

#endif // !SURFACE_H
