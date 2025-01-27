#include <stdexcept>

#include <iostream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Texture.h"

#include "DataBuffer.h"
#include "VulkanUtils.h"
#include "VulkanInstance.h"

Texture::Texture()
	: m_Image{}
	, m_ImageView{}
	, m_TextureSampler{}
{
}

void Texture::Initialize(const VulkanInstance& instance, const CommandPool& cmndPl, const std::string& filePath)
{
	const VkDevice& device{ instance.GetVkDevice() };
	const VkPhysicalDevice& phyDevice{ instance.GetVkPhysicalDevice() };
	const VkQueue& graphQ{ instance.GetGraphicsQueue() };

	constexpr VkFormat imageFormat{ VK_FORMAT_R8G8B8A8_SRGB };

	// Image //
	InitImage(device, phyDevice, graphQ, cmndPl, filePath, imageFormat);

	// ImageView //
	InitImageView(device, imageFormat);

	// Sampler
	m_TextureSampler.Initialize(device, phyDevice);
}

void Texture::Initialize(const VulkanInstance& instance, const CommandPool& cmndPl, const std::string& filePath, const Sampler& sampler)
{
	const VkDevice& device{ instance.GetVkDevice() };
	const VkPhysicalDevice& phyDevice{ instance.GetVkPhysicalDevice() };
	const VkQueue& graphQ{ instance.GetGraphicsQueue() };

	constexpr VkFormat imageFormat{ VK_FORMAT_R8G8B8A8_SRGB };

	// Image //
	InitImage(device, phyDevice, graphQ, cmndPl, filePath, imageFormat);

	// ImageView //
	InitImageView(device, imageFormat);
	m_TextureSampler.Destroy(device);
	m_TextureSampler = sampler;
}

void Texture::Destroy(VkDevice device)
{
	m_TextureSampler.Destroy(device);
	m_ImageView.Destroy(device);
	m_Image.Destroy(device);
}

const VkImage& Texture::GetVkImage() const
{
	return m_Image.GetVkImage();
}

const Image& Texture::GetImage() const
{
	return m_Image;
}

const VkImageView& Texture::GetVkImageView() const
{
	return m_ImageView.GetVkImageView();
}

const ImageView& Texture::GetImageView() const
{
	return m_ImageView;
}

const VkSampler& Texture::GetVkSampler() const
{
	return m_TextureSampler.GetVkSampler();
}

const Sampler& Texture::GetSampler() const
{
	return m_TextureSampler;
}

void Texture::InitImage(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& cmndPl, const std::string& filePath, VkFormat imageFormat)
{
	if (!std::filesystem::exists(filePath))
	{
		throw std::runtime_error("Image file does not exist: " + filePath);
	}

	constexpr VkImageTiling imageTilling{ VK_IMAGE_TILING_OPTIMAL };
	constexpr VkImageUsageFlags imageUsage{ VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT };
	constexpr VkMemoryPropertyFlags imageProperties{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

	constexpr VkImageLayout oldLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
	constexpr VkImageLayout newerLayout{ VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL };
	constexpr VkImageLayout newestLayout{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

	constexpr VkMemoryPropertyFlags stagingBufferProperties{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
	constexpr VkBufferUsageFlags stagingBufferUsage{ VK_BUFFER_USAGE_TRANSFER_SRC_BIT };

	int texWidth{};
	int texHeight{};
	int texChannels{};

	stbi_uc* pixels{ stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha) };
	const VkDeviceSize imageSize{ static_cast<uint64_t>(texWidth * texHeight * 4) };

	if (!pixels) throw std::runtime_error("failed to load texture image: " + std::string(stbi_failure_reason()));

	DataBuffer stagingBuffer{};
	stagingBuffer.Initialize(device, phyDevice, stagingBufferProperties, imageSize, stagingBufferUsage);
	stagingBuffer.Upload(device, imageSize, pixels);

	stbi_image_free(pixels);

	m_Image.Initialize(device, phyDevice, texWidth, texHeight, imageFormat, imageTilling, imageUsage, imageProperties);

	m_Image.TransitionImageLayout(device, cmndPl, queue, imageFormat, oldLayout, newerLayout);
	m_Image.CopyBufferToImage(device, stagingBuffer, cmndPl, queue);
	m_Image.TransitionImageLayout(device, cmndPl, queue, imageFormat, newerLayout, newestLayout);

	stagingBuffer.Destroy(device);
}

void Texture::InitImageView(VkDevice device, VkFormat imageFormat)
{
	m_ImageView.Initialize(device, m_Image.GetVkImage(), imageFormat);
}