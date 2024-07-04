#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <vulkan/vulkan.h>

class RenderPass final
{
public:

	RenderPass();
	~RenderPass() = default;

	RenderPass(const RenderPass& other) = delete;
	RenderPass(RenderPass&& other) noexcept = delete;
	RenderPass& operator=(const RenderPass& other) = delete;
	RenderPass& operator=(RenderPass&& other) noexcept = delete;

	void Initialize(VkDevice device, VkFormat swapchainImageFormat, VkFormat depthFormat);
	void Destroy(VkDevice device);

	const VkRenderPass& GetVkRenderPass() const;

private:

	VkRenderPass m_RenderPass;

};

#endif // !RENDERPASS_H