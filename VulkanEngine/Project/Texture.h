#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

#include <vulkan/vulkan.h>

#include "Image.h"
#include "ImageView.h"

class CommandPool;

class Texture final
{
public:

	Texture();
	~Texture() = default;

	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, const CommandPool& cmndPl, VkQueue queue, const std::string& filePath);
	void Destroy(VkDevice device);

	const Image& GetImage() const;
	const ImageView& GetImageView() const;

private:

	void InitImage(VkDevice device, VkPhysicalDevice phyDevice, const CommandPool& cmndPl, VkQueue queue, const std::string& filePath, VkFormat imageFormat);
	void InitImageView(VkDevice device, VkFormat imageFormat);

private:

	Image m_Image;
	ImageView m_ImageView;

};

#endif // !TEXTURE_H
