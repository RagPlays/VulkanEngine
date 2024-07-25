#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

#include <vulkan/vulkan.h>

#include "Image.h"
#include "ImageView.h"
#include "Sampler.h"

class CommandPool;
class VulkanInstance;

class Texture final
{
public:

	Texture();
	~Texture() = default;

	void Initialize(const VulkanInstance& instance, const CommandPool& cmndPl, const std::string& filePath);
	void Initialize(const VulkanInstance& instance, const CommandPool& cmndPl, const std::string& filePath, const Sampler& sampler);
	void Destroy(VkDevice device);

	const VkImage& GetVkImage() const;
	const Image& GetImage() const;
	const VkImageView& GetVkImageView() const;
	const ImageView& GetImageView() const;
	const VkSampler& GetVkSampler() const;
	const Sampler& GetSampler() const;

private:

	void InitImage(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& cmndPl, const std::string& filePath, VkFormat imageFormat);
	void InitImageView(VkDevice device, VkFormat imageFormat);

private:

	Image m_Image;
	ImageView m_ImageView;
	Sampler m_TextureSampler;

};

#endif // !TEXTURE_H
