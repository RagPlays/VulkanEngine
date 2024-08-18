#ifndef WINDOW_H
#define WINDOW_H

#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

// 720p resolution (AR 16:9)
constexpr uint32_t g_WindowStartWidth{ 1920 };
constexpr uint32_t g_WindowStartHeight{ 1080 };

class Window final
{
public:

	Window();
	~Window() = default;

	Window(const Window& other) = delete;
	Window(Window&& other) noexcept = delete;
	Window& operator=(const Window& other) = delete;
	Window& operator=(Window&& other) noexcept = delete;

	void Initialize();
	void Destroy();

	bool WindowShouldClose() const;

	GLFWwindow* GetWindow() const;

	void GetFramebufferSize(int& width, int& height) const;

	void PollEvents() const;
	void WaitEvents() const;

	float GetAspectRatio() const;
	float GetStartAspectRatio() const;

	bool GetFramebufferResized() const;
	void SetFramebufferResized(bool resized);

private:

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

private:

	GLFWwindow* m_pWindow;

	bool m_FramebufferResized;

	static const std::vector<std::pair<int, int>> s_WindowHints;
};

#endif // !WINDOW_H
