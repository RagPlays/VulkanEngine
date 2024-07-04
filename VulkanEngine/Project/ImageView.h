#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <vulkan/vulkan.h>

class ImageView final
{
public:

	ImageView() = default;
	~ImageView() = default;

	void Initialize(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
	void Destroy(VkDevice device);

	const VkImageView& GetVkImageView() const;

private:

	VkImageView m_VkImageView;

};

#endif // !IMAGEVIEW_H