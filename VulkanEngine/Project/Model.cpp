#include <iostream>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Model.h"
#include "VulkanUtils.h"
#include "Camera.h"
#include "VulkanInstance.h"

// MODEL 2D //
Model2D::Model2D()
    : m_Transform{}
    , m_ModelMatrix{}
    , m_NrIndices {}
    , m_VertexBuffer{}
    , m_IndexBuffer{}
{
}

void Model2D::Initialize(const VulkanInstance& instance, const CommandPool& cmndP, const std::string& modelFilePath)
{
    m_NrIndices = 0;
    std::vector<Vertex2D> vertices{};
    std::vector<uint32_t> indices{};

    LoadModelFromFile(modelFilePath, vertices, indices);
    InitDataBuffers(instance, cmndP, vertices, indices);
    UpdateModelMatrix();
}

void Model2D::Initialize(const VulkanInstance& instance, const CommandPool& cmndP, const std::vector<Vertex2D>& vertices, const std::vector<uint32_t>& indices)
{
    m_NrIndices = static_cast<uint32_t>(indices.size());
    InitDataBuffers(instance, cmndP, vertices, indices);
    UpdateModelMatrix();
}

void Model2D::Destroy(VkDevice device)
{
    m_VertexBuffer.Destroy(device);
    m_IndexBuffer.Destroy(device);
}

void Model2D::SetPosition(const glm::vec2& pos)
{
    m_Transform.position = pos;
    UpdateModelMatrix();
}

void Model2D::SetRotation(float angle)
{
    m_Transform.rotation = glm::radians(angle);
    UpdateModelMatrix();
}

void Model2D::SetScale(const glm::vec2& scale)
{
    m_Transform.scale = scale;
    UpdateModelMatrix();
}

void Model2D::SetScale(float scale)
{
    SetScale(glm::vec2{ scale, scale });
    UpdateModelMatrix();
}

void Model2D::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelUBO), &m_ModelMatrix);

    m_VertexBuffer.BindAsVertexBuffer(commandBuffer);
    m_IndexBuffer.BindAsIndexBuffer(commandBuffer);

    vkCmdDrawIndexed(commandBuffer, m_NrIndices, 1, 0, 0, 0);
}

void Model2D::LoadModelFromFile(const std::string& filePath, std::vector<Vertex2D>& vertices, std::vector<uint32_t>& indices)
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

    std::unordered_map<Vertex2D, uint32_t> uniqueVertices{};
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            // Setting vertex
            Vertex2D vertex{};

            vertex.pos =
            {
                static_cast<float>(attrib.vertices[3 * index.vertex_index + 0]),
                static_cast<float>(attrib.vertices[3 * index.vertex_index + 1])
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
    m_NrIndices = static_cast<uint32_t>(indices.size());
}

void Model2D::InitDataBuffers(const VulkanInstance& instance, const CommandPool& commandPool, const std::vector<Vertex2D>& vertices, const std::vector<uint32_t>& indices)
{
    const VkDevice& device{ instance.GetVkDevice() };
    const VkPhysicalDevice& phyDevice{ instance.GetVkPhysicalDevice() };
    const VkQueue& graphQ{ instance.GetGraphicsQueue() };

    constexpr VkBufferUsageFlags stagingBufferUsage{ VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
    constexpr VkMemoryPropertyFlags stagingBufferProperties{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    constexpr VkMemoryPropertyFlags bufferProperties{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

    /////// Vertex Buffer ///////
    const VkDeviceSize vertexBufferSize{ sizeof(vertices[0]) * vertices.size() };
    constexpr VkBufferUsageFlags vertexBufferUsage{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT };

    DataBuffer stagingVBuffer{};
    stagingVBuffer.Initialize(device, phyDevice, stagingBufferProperties, vertexBufferSize, stagingBufferUsage);
    stagingVBuffer.Upload(device, vertexBufferSize, vertices.data());

    m_VertexBuffer.Initialize(device, phyDevice, bufferProperties, vertexBufferSize, vertexBufferUsage);
    DataBuffer::CopyBuffer(graphQ, device, commandPool, stagingVBuffer, m_VertexBuffer, vertexBufferSize);

    /////// Index Buffer ///////
    const VkDeviceSize indexBufferSize{ sizeof(indices[0]) * indices.size() };
    constexpr VkBufferUsageFlags indexBufferUsage{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT };

    DataBuffer stagingIBuffer{};
    stagingIBuffer.Initialize(device, phyDevice, stagingBufferProperties, indexBufferSize, stagingBufferUsage);
    stagingIBuffer.Upload(device, indexBufferSize, indices.data());

    m_IndexBuffer.Initialize(device, phyDevice, bufferProperties, indexBufferSize, indexBufferUsage);
    DataBuffer::CopyBuffer(graphQ, device, commandPool, stagingIBuffer, m_IndexBuffer, indexBufferSize);

    // Destroy Staging Buffers //
    stagingVBuffer.Destroy(device);
    stagingIBuffer.Destroy(device);
}

void Model2D::UpdateModelMatrix()
{
    m_ModelMatrix.model = m_Transform.GetModelMatrix();
}

// MODEL 3D //
Model3D::Model3D()
    : m_Transform{}
    , m_ModelMatrix{}
    , m_NrIndices{}
    , m_VertexBuffer{}
    , m_IndexBuffer{}
{
}

void Model3D::Initialize(const VulkanInstance& instance, const CommandPool& commandPool, const std::string& modelFilePath)
{
    m_NrIndices = 0;
    std::vector<Vertex3D> vertices{};
    std::vector<uint32_t> indices{};

    LoadModelFromFile(modelFilePath, vertices, indices);
    InitDataBuffers(instance, commandPool, vertices, indices);
    UpdateModelMatrix();
}

void Model3D::Initialize(const VulkanInstance& instance, const CommandPool& cmndP, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices)
{
    m_NrIndices = static_cast<uint32_t>(indices.size());
    InitDataBuffers(instance, cmndP, vertices, indices);
    UpdateModelMatrix();
}

void Model3D::Destroy(VkDevice device)
{
    m_VertexBuffer.Destroy(device);
    m_IndexBuffer.Destroy(device);
}

void Model3D::SetPosition(const glm::vec3& position)
{
    m_Transform.position = position;
    UpdateModelMatrix();
}

void Model3D::SetRotation(const glm::quat& rotation)
{
    m_Transform.rotation = rotation;
    UpdateModelMatrix();
}

void Model3D::SetScale(const glm::vec3& scale)
{
    m_Transform.scale = scale;
    UpdateModelMatrix();
}

void Model3D::SetScale(float scale)
{
    SetScale(glm::vec3{ scale, scale, scale });
}

void Model3D::SetTranform(const Transform3D& transform)
{
    m_Transform = transform;
    UpdateModelMatrix();
}

void Model3D::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelUBO), &m_ModelMatrix);

    m_VertexBuffer.BindAsVertexBuffer(commandBuffer);
    m_IndexBuffer.BindAsIndexBuffer(commandBuffer);

    vkCmdDrawIndexed(commandBuffer, m_NrIndices, 1, 0, 0, 0);
}

void Model3D::LoadModelFromFile(const std::string& filePath, std::vector<Vertex3D>& vertices, std::vector<uint32_t>& indices)
{
    tinyobj::attrib_t attrib{};
    std::vector<tinyobj::shape_t> shapes{};
    std::vector<tinyobj::material_t> materials{};
    std::string warn{};
    std::string err{};
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str()))
    {
        throw std::runtime_error{ warn + err };
    }

    std::unordered_map<Vertex3D, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex3D vertex{};

            // Position
            vertex.pos =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            // Texture coordinates
            if (index.texcoord_index >= 0)
            {
                vertex.texCoord =
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            //// Normals
            //if (index.normal_index >= 0)
            //{
            //    vertex.normal =
            //    {
            //        attrib.normals[3 * index.normal_index + 0],
            //        attrib.normals[3 * index.normal_index + 1],
            //        attrib.normals[3 * index.normal_index + 2]
            //    };
            //}

            // Colors
            if (!attrib.colors.empty())
            {
                vertex.color =
                {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2]
                };
            }
            else vertex.color = { 1.0f, 1.0f, 1.0f }; // Default white if no color data

            // Add vertex to the list
            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.emplace_back(vertex);
            }

            indices.emplace_back(uniqueVertices[vertex]);
        }
    }

    m_NrIndices = static_cast<uint32_t>(indices.size());
}

void Model3D::InitDataBuffers(const VulkanInstance& instance, const CommandPool& commandPool, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices)
{
    const VkDevice& device{ instance.GetVkDevice() };
    const VkPhysicalDevice& phyDevice{ instance.GetVkPhysicalDevice() };
    const VkQueue& graphQ{ instance.GetGraphicsQueue() };

    constexpr VkBufferUsageFlags stagingBufferUsage{ VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
    constexpr VkMemoryPropertyFlags stagingBufferProperties{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    constexpr VkMemoryPropertyFlags bufferProperties{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

    /////// Vertex Buffer ///////
    const VkDeviceSize vertexBufferSize{ sizeof(vertices[0]) * vertices.size() };
    constexpr VkBufferUsageFlags vertexBufferUsage{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT };

    DataBuffer stagingVBuffer{};
    stagingVBuffer.Initialize(device, phyDevice, stagingBufferProperties, vertexBufferSize, stagingBufferUsage);
    stagingVBuffer.Upload(device, vertexBufferSize, vertices.data());

    m_VertexBuffer.Initialize(device, phyDevice, bufferProperties, vertexBufferSize, vertexBufferUsage);
    DataBuffer::CopyBuffer(graphQ, device, commandPool, stagingVBuffer, m_VertexBuffer, vertexBufferSize);

    /////// Index Buffer ///////
    const VkDeviceSize indexBufferSize{ sizeof(indices[0]) * indices.size() };
    constexpr VkBufferUsageFlags indexBufferUsage{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT };

    DataBuffer stagingIBuffer{};
    stagingIBuffer.Initialize(device, phyDevice, stagingBufferProperties, indexBufferSize, stagingBufferUsage);
    stagingIBuffer.Upload(device, indexBufferSize, indices.data());

    m_IndexBuffer.Initialize(device, phyDevice, bufferProperties, indexBufferSize, indexBufferUsage);
    DataBuffer::CopyBuffer(graphQ, device, commandPool, stagingIBuffer, m_IndexBuffer, indexBufferSize);

    // Destroy staging buffers //
    stagingVBuffer.Destroy(device);
    stagingIBuffer.Destroy(device);
}

void Model3D::UpdateModelMatrix()
{
    m_ModelMatrix.model = m_Transform.GetModelMatrix();
}

// MODEL3DIR //

Model3DIR::Model3DIR()
    : m_Transforms{}
    , m_ModelMatrices{}
    , m_NrIndices{}
    , m_InstanceCount{}
    , m_VertexBuffer{}
    , m_IndexBuffer{}
    , m_InstanceBuffer{}
{
}

void Model3DIR::Initialize(const VulkanInstance& instance, const CommandPool& commandPool, const std::string& modelFilePath, uint32_t instanceCount)
{
    if (instanceCount < 1) throw std::exception{ "Model: invalid instanceCount value!" };

    m_InstanceCount = instanceCount;
    m_Transforms.resize(instanceCount);
    m_ModelMatrices.resize(instanceCount);

    m_NrIndices = 0;
    std::vector<Vertex3DIR> vertices{};
    std::vector<uint32_t> indices{};

    LoadModelFromFile(modelFilePath, vertices, indices);
    InitDataBuffers(instance, commandPool, vertices, indices);
    UpdateModelBuffer(instance.GetVkDevice());
}

void Model3DIR::Initialize(const VulkanInstance& instance, const CommandPool& cmndP, const std::vector<Vertex3DIR>& vertices, const std::vector<uint32_t>& indices, uint32_t instanceCount)
{
    if (instanceCount < 1) throw std::exception{ "Model: invalid instanceCount value!" };

    m_InstanceCount = instanceCount;
    m_Transforms.resize(instanceCount);
    m_ModelMatrices.resize(instanceCount);

    m_NrIndices = static_cast<uint32_t>(indices.size());
    InitDataBuffers(instance, cmndP, vertices, indices);
    for (uint32_t instanceIdx{}; instanceIdx < instanceCount; ++instanceIdx)
    {
        UpdateModelMatrix(instanceIdx);
    }
}

void Model3DIR::Destroy(VkDevice device)
{
    // Destroy Vulkan buffers
    m_VertexBuffer.Destroy(device);
    m_IndexBuffer.Destroy(device);
    m_InstanceBuffer.Destroy(device);

    // Clear model matrices and transforms
    m_Transforms.clear();
    m_ModelMatrices.clear();
}

void Model3DIR::SetPosition(const glm::vec3& position)
{
    for (size_t idx{}; idx < m_ModelMatrices.size(); ++idx)
    {
        SetPosition(static_cast<uint32_t>(idx), position);
    }
}

void Model3DIR::SetPosition(uint32_t instanceIndex, const glm::vec3& position)
{
    if (instanceIndex < m_Transforms.size())
    {
        m_Transforms[instanceIndex].position = position;
        UpdateModelMatrix(instanceIndex);
    }
}

void Model3DIR::SetRotation(const glm::vec3& rotation)
{
    for (size_t idx{}; idx < m_ModelMatrices.size(); ++idx)
    {
        SetRotation(static_cast<uint32_t>(idx), rotation);
    }
}

void Model3DIR::SetRotation(uint32_t instanceIndex, const glm::vec3& rotation)
{
    if (instanceIndex < m_Transforms.size())
    {
        m_Transforms[instanceIndex].rotation = glm::quat{ rotation };
        UpdateModelMatrix(instanceIndex);
    }
}

void Model3DIR::SetScale(const glm::vec3& scale)
{
    for (size_t idx{}; idx < m_ModelMatrices.size(); ++idx)
    {
        SetScale(static_cast<uint32_t>(idx), scale);
    }
}

void Model3DIR::SetScale(float scale)
{
    for (size_t idx{}; idx < m_ModelMatrices.size(); ++idx)
    {
        SetScale(static_cast<uint32_t>(idx), scale);
    }
}

void Model3DIR::SetScale(uint32_t instanceIndex, const glm::vec3& scale)
{
    if (instanceIndex < m_Transforms.size())
    {
        m_Transforms[instanceIndex].scale = scale;
        UpdateModelMatrix(instanceIndex);
    }
}

void Model3DIR::SetScale(uint32_t instanceIndex, float scale)
{
    SetScale(instanceIndex, glm::vec3{ scale, scale, scale });
}

void Model3DIR::SetTransform(uint32_t instanceIndex, const Transform3D& transform)
{
    if (instanceIndex < m_Transforms.size())
    {
        m_Transforms[instanceIndex] = transform;
        UpdateModelMatrix(instanceIndex);
    }
}

uint32_t Model3DIR::GetInstanceCount() const
{
    return m_InstanceCount;
}

void Model3DIR::Update(VkDevice device)
{
    UpdateModelBuffer(device);
}

void Model3DIR::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
    m_VertexBuffer.BindAsVertexBuffer(commandBuffer);
    m_InstanceBuffer.BindAsVertexBuffer(commandBuffer, 1);

    m_IndexBuffer.BindAsIndexBuffer(commandBuffer);

    vkCmdDrawIndexed(commandBuffer, m_NrIndices, m_InstanceCount, 0, 0, 0);
}

void Model3DIR::LoadModelFromFile(const std::string& filePath, std::vector<Vertex3DIR>& vertices, std::vector<uint32_t>& indices)
{
    tinyobj::attrib_t attrib{};
    std::vector<tinyobj::shape_t> shapes{};
    std::vector<tinyobj::material_t> materials{};
    std::string warn{};
    std::string err{};
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str()))
    {
        throw std::runtime_error{ warn + err };
    }

    std::unordered_map<Vertex3DIR, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex3DIR vertex{};

            // Position
            vertex.pos =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            // Texture coordinates
            if (index.texcoord_index >= 0)
            {
                vertex.texCoord =
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            //// Normals
            //if (index.normal_index >= 0)
            //{
            //    vertex.normal =
            //    {
            //        attrib.normals[3 * index.normal_index + 0],
            //        attrib.normals[3 * index.normal_index + 1],
            //        attrib.normals[3 * index.normal_index + 2]
            //    };
            //}

            // Colors
            if (!attrib.colors.empty())
            {
                vertex.color =
                {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2]
                };
            }
            else vertex.color = { 1.0f, 1.0f, 1.0f }; // Default white if no color data

            // Add vertex to the list
            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.emplace_back(vertex);
            }

            indices.emplace_back(uniqueVertices[vertex]);
        }
    }

    m_NrIndices = static_cast<uint32_t>(indices.size());
}

void Model3DIR::InitDataBuffers(const VulkanInstance& instance, const CommandPool& commandPool, const std::vector<Vertex3DIR>& vertices, const std::vector<uint32_t>& indices)
{
    const VkDevice& device = instance.GetVkDevice();
    const VkPhysicalDevice& phyDevice = instance.GetVkPhysicalDevice();
    const VkQueue& graphQ = instance.GetGraphicsQueue();

    constexpr VkBufferUsageFlags stagingBufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    constexpr VkMemoryPropertyFlags stagingBufferProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    constexpr VkMemoryPropertyFlags bufferProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    // Vertex Buffer
    VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
    constexpr VkBufferUsageFlags vertexBufferUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    DataBuffer stagingVBuffer{};
    stagingVBuffer.Initialize(device, phyDevice, stagingBufferProperties, vertexBufferSize, stagingBufferUsage);
    stagingVBuffer.Upload(device, vertexBufferSize, vertices.data());

    m_VertexBuffer.Initialize(device, phyDevice, bufferProperties, vertexBufferSize, vertexBufferUsage);
    DataBuffer::CopyBuffer(graphQ, device, commandPool, stagingVBuffer, m_VertexBuffer, vertexBufferSize);

    // Index Buffer
    VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
    constexpr VkBufferUsageFlags indexBufferUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    DataBuffer stagingIBuffer{};
    stagingIBuffer.Initialize(device, phyDevice, stagingBufferProperties, indexBufferSize, stagingBufferUsage);
    stagingIBuffer.Upload(device, indexBufferSize, indices.data());

    m_IndexBuffer.Initialize(device, phyDevice, bufferProperties, indexBufferSize, indexBufferUsage);
    DataBuffer::CopyBuffer(graphQ, device, commandPool, stagingIBuffer, m_IndexBuffer, indexBufferSize);

    // Instance Buffer
    VkDeviceSize instanceBufferSize = sizeof(ModelUBO) * m_InstanceCount;
    constexpr VkBufferUsageFlags instanceBufferUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    DataBuffer stagingInstanceBuffer{};
    stagingInstanceBuffer.Initialize(device, phyDevice, stagingBufferProperties, instanceBufferSize, stagingBufferUsage);
    stagingInstanceBuffer.Upload(device, instanceBufferSize, m_ModelMatrices.data());

    m_InstanceBuffer.Initialize(device, phyDevice, stagingBufferProperties, instanceBufferSize, instanceBufferUsage);
    DataBuffer::CopyBuffer(graphQ, device, commandPool, stagingInstanceBuffer, m_InstanceBuffer, instanceBufferSize);

    // Destroy staging buffers
    stagingVBuffer.Destroy(device);
    stagingIBuffer.Destroy(device);
    stagingInstanceBuffer.Destroy(device);
}

void Model3DIR::UpdateModelMatrix(uint32_t instanceIndex)
{
    if (instanceIndex < m_ModelMatrices.size())
    {
        m_ModelMatrices[instanceIndex].model = m_Transforms[instanceIndex].GetModelMatrix();
    }
}

void Model3DIR::UpdateModelBuffer(VkDevice device) const
{
    const VkDeviceSize& bufferSize{ m_InstanceBuffer.GetSizeInBytes() };
    m_InstanceBuffer.Upload(device, bufferSize, m_ModelMatrices.data());
}