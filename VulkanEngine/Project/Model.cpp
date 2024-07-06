#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <Libraries/tiny_obj_loader.h>

#include "Model.h"
#include "VulkanUtils.h"
#include "Camera.h"

void Model::Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& commandPool, const std::string& filePath)
{
    m_NrIndices = 0;
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};

    LoadModelFromFile(filePath, vertices, indices);
    InitDataBuffers(device, phyDevice, queue, commandPool, vertices, indices);
}

void Model::Destroy(VkDevice device)
{
    m_VertexBuffer.Destroy(device);
    m_IndexBuffer.Destroy(device);
}

void Model::SetPosition(glm::vec3 position)
{
    m_Transform.position = position;
    UpdateModelMatrix();
}

void Model::SetRotation(glm::quat rotation)
{
    m_Transform.rotation = rotation;
    UpdateModelMatrix();
}

void Model::SetScale(glm::vec3 scale)
{
    m_Transform.scale = scale;
    UpdateModelMatrix();
}

void Model::SetTranform(const Transform& transform)
{
    m_Transform = transform;
    UpdateModelMatrix();
}

void Model::Draw(VkCommandBuffer commandBuffer, Camera* camera) const
{
    m_VertexBuffer.BindAsVertexBuffer(commandBuffer);
    m_IndexBuffer.BindAsIndexBuffer(commandBuffer);

    vkCmdDrawIndexed(commandBuffer, m_NrIndices, 1, 0, 0, 0);
}

void Model::LoadModelFromFile(const std::string& filePath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    tinyobj::attrib_t attrib{};
    std::vector<tinyobj::shape_t> shapes{};
    std::vector<tinyobj::material_t> materials{};
    std::string warn{};
    std::string err{};

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            // Setting vertex
            Vertex vertex{};

            vertex.pos =
            {
                static_cast<float>(attrib.vertices[3 * index.vertex_index + 0]),
                static_cast<float>(attrib.vertices[3 * index.vertex_index + 1]),
                static_cast<float>(attrib.vertices[3 * index.vertex_index + 2])
            };

            vertex.texCoord =
            {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            // Pushing vertex
            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.emplace_back(vertex);
            }
            indices.emplace_back(uniqueVertices[vertex]);
        }
    }
    m_NrIndices = indices.size();
}

void Model::InitDataBuffers(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& commandPool, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    /////// Vertex Buffer ///////
    const VkDeviceSize vertexBufferSize{ sizeof(vertices[0]) * vertices.size() };

    const VkBufferUsageFlags stagingBufferUsage{ VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
    const VkMemoryPropertyFlags stagingBufferProperties{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    const VkMemoryPropertyFlags bufferProperties{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

    // staging buffer //
    DataBuffer stagingVBuffer{};
    stagingVBuffer.Initialize(device, phyDevice, stagingBufferProperties, vertexBufferSize, stagingBufferUsage);
    stagingVBuffer.Upload(device, vertexBufferSize, vertices.data());

    // vertex buffer //
    const VkBufferUsageFlags vertexBufferUsage{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT };

    m_VertexBuffer.Initialize(device, phyDevice, bufferProperties, vertexBufferSize, vertexBufferUsage);
    DataBuffer::CopyBuffer(queue, device, commandPool, stagingVBuffer, m_VertexBuffer, vertexBufferSize);

    stagingVBuffer.Destroy(device);

    /////// Index Buffer ///////
    const VkDeviceSize indexBufferSize{ sizeof(indices[0]) * indices.size() };

    // staging buffer //
    DataBuffer stagingIBuffer{};
    stagingIBuffer.Initialize(device, phyDevice, stagingBufferProperties, indexBufferSize, stagingBufferUsage);
    stagingIBuffer.Upload(device, indexBufferSize, indices.data());

    // index buffer //
    const VkBufferUsageFlags indexBufferUsage{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT };

    m_IndexBuffer.Initialize(device, phyDevice, bufferProperties, indexBufferSize, indexBufferUsage);
    DataBuffer::CopyBuffer(queue, device, commandPool, stagingIBuffer, m_IndexBuffer, indexBufferSize);

    stagingIBuffer.Destroy(device);
}

void Model::UpdateModelMatrix()
{
    m_ModelMatrix = m_Transform.GetModelMatrix();
}