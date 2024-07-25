#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <vulkan/vulkan.h>

class VulkanInstance;
class Swapchain;
class DepthBuffer;

class RenderPass final
{
public:

	RenderPass();
	~RenderPass() = default;

	RenderPass(const RenderPass& other) = delete;
	RenderPass(RenderPass&& other) noexcept = delete;
	RenderPass& operator=(const RenderPass& other) = delete;
	RenderPass& operator=(RenderPass&& other) noexcept = delete;

	void Initialize(const VulkanInstance& instance, const Swapchain& swapchain, const DepthBuffer& depthBuffer);
	void Destroy(VkDevice device);

	const VkRenderPass& GetVkRenderPass() const;

private:

	VkRenderPass m_VkRenderPass;

};

#endif // !RENDERPASS_H