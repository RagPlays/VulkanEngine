#include <stdexcept>

#include "Image.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "DataBuffer.h"

Image::Image()
    : m_VkDevice{ VK_NULL_HANDLE }
    , m_Width{}
    , m_Heigth{}
    , m_VkImage{ VK_NULL_HANDLE }
    , m_VkImageMemory{ VK_NULL_HANDLE }
{
}

void Image::Initialize(VkDevice device, VkPhysicalDevice phyDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags prop)
{
    m_VkDevice = device;
    m_Width = width;
    m_Heigth = height;

    // Creating Image //
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &m_VkImage) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(device, m_VkImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(phyDevice, memRequirements.memoryTypeBits, prop);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &m_VkImageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, m_VkImage, m_VkImageMemory, 0);
}

void Image::Destroy()
{
    if (m_VkDevice == VK_NULL_HANDLE) return;
    if (m_VkImage != VK_NULL_HANDLE) vkDestroyImage(m_VkDevice, m_VkImage, nullptr);
    if (m_VkImageMemory != VK_NULL_HANDLE) vkFreeMemory(m_VkDevice, m_VkImageMemory, nullptr);
    m_VkImage = VK_NULL_HANDLE;
    m_VkImageMemory = VK_NULL_HANDLE;
}

void Image::TransitionImageLayout(const CommandPool& commandPool, VkQueue queue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    CommandBuffer commandBuffer{ commandPool.CreateCommandBuffer(m_VkDevice) };
    commandBuffer.BeginRecording();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = m_VkImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;


    VkPipelineStageFlags sourceStage{};
    VkPipelineStageFlags destinationStage{};

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (HasStencilComponent(format)) 
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier
    (
        commandBuffer.GetVkCommandBuffer(),
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    commandBuffer.EndRecording();

    commandBuffer.Submit(queue, VK_NULL_HANDLE);
    commandBuffer.Destroy(m_VkDevice, commandPool);
}

void Image::CopyBufferToImage(const DataBuffer& buffer, const CommandPool& commandPool, VkQueue queue)
{
    CommandBuffer commandBuffer{ commandPool.CreateCommandBuffer(m_VkDevice) };
    commandBuffer.BeginRecording();
    {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            m_Width,
            m_Heigth,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer.GetVkCommandBuffer(),
            buffer.GetVkBuffer(),
            m_VkImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    }
    commandBuffer.EndRecording();

    commandBuffer.Submit(queue, VK_NULL_HANDLE);
    commandBuffer.Destroy(m_VkDevice, commandPool);
}

bool Image::HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

const VkImage& Image::GetVkImage() const
{
	return m_VkImage;
}

const VkDeviceMemory& Image::GetVkDeviceMemory() const
{
	return m_VkImageMemory;
}

uint32_t Image::GetWidth() const
{
    return m_Width;
}

uint32_t Image::GetHeight() const
{
    return m_Heigth;
}

uint32_t Image::FindMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(physDevice, &memProperties);

    for (uint32_t idx{}; idx < memProperties.memoryTypeCount; ++idx)
    {
        if ((typeFilter & (1 << idx)) && (memProperties.memoryTypes[idx].propertyFlags & properties) == properties)
        {
            return idx;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}