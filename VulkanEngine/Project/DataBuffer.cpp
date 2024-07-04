#include <stdexcept>

#include "DataBuffer.h"
#include "CommandPool.h"

DataBuffer::DataBuffer()
	: m_Size{ 0 }
    , m_VkBuffer{ VK_NULL_HANDLE }
    , m_VkBufferMemory{ VK_NULL_HANDLE }
{
}

void DataBuffer::Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkMemoryPropertyFlags prop, VkDeviceSize size, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    Initialize(device, phyDevice, prop, bufferInfo);
}

void DataBuffer::Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkMemoryPropertyFlags prop, const VkBufferCreateInfo& bufferCreateInfo)
{
    if (m_VkBuffer != VK_NULL_HANDLE) return;
    m_Size = bufferCreateInfo.size;

    // BUFFER CREATION //
    if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, &m_VkBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    // MEMORY ALLOCATION //
    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(device, m_VkBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(phyDevice, memRequirements.memoryTypeBits, prop);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &m_VkBufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(device, m_VkBuffer, m_VkBufferMemory, 0);
}

void DataBuffer::Destroy(VkDevice device)
{
    if (m_VkBuffer != VK_NULL_HANDLE) vkDestroyBuffer(device, m_VkBuffer, nullptr);
    if (m_VkBufferMemory != VK_NULL_HANDLE) vkFreeMemory(device, m_VkBufferMemory, nullptr);
    m_VkBuffer = VK_NULL_HANDLE;
    m_VkBufferMemory = VK_NULL_HANDLE;
}

const VkBuffer& DataBuffer::GetVkBuffer() const
{
	return m_VkBuffer;
}

const VkDeviceMemory& DataBuffer::GetVkDeviceMemory() const
{
    return m_VkBufferMemory;
}

const VkDeviceSize& DataBuffer::GetSizeInBytes() const
{
    return m_Size;
}

void DataBuffer::Upload(VkDevice device, VkDeviceSize size, const void* data)
{
    void* mappedData{};
    //vkMapMemory(device, m_VkBufferMemory, 0, size, 0, &mappedData);
    Map(device, size, &mappedData);
    if(mappedData) memcpy(mappedData, data, static_cast<size_t>(size));
    vkUnmapMemory(device, m_VkBufferMemory);
}

void DataBuffer::Map(VkDevice device, VkDeviceSize size, void** data)
{
    vkMapMemory(device, m_VkBufferMemory, 0, size, 0, data);
}

void DataBuffer::BindAsVertexBuffer(VkCommandBuffer commandBuffer)
{
    VkBuffer vertexBuffers[]{ m_VkBuffer };
    VkDeviceSize offsets[]{ 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
}

void DataBuffer::BindAsIndexBuffer(VkCommandBuffer commandBuffer)
{
    vkCmdBindIndexBuffer(commandBuffer, m_VkBuffer, 0, VK_INDEX_TYPE_UINT16);
}

void DataBuffer::CopyBuffer(VkQueue graphicsQueue, VkDevice device, const CommandPool& commandPool, DataBuffer srcBuffer, DataBuffer dstBuffer, VkDeviceSize size)
{
    CommandBuffer commandBuffer{ commandPool.CreateCommandBuffer(device) };

    commandBuffer.BeginRecording();
    {
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer.GetVkCommandBuffer(), srcBuffer.GetVkBuffer(), dstBuffer.GetVkBuffer(), 1, &copyRegion);
    }
    commandBuffer.EndRecording();

    commandBuffer.Submit(graphicsQueue, VK_NULL_HANDLE);

    commandBuffer.Destroy(device, commandPool);
}

uint32_t DataBuffer::FindMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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