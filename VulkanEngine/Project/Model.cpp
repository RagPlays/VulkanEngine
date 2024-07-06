#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <Libraries/tiny_obj_loader.h>

//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"
#include "VulkanUtils.h"
#include "Camera.h"

Model::Model()
    : m_ModelUBOs{}
    , m_ModelUBOsMapped{}
    , m_Transform{}
    , m_ModelMatrix{}
    , m_NrIndices{}
    , m_VertexBuffer{}
    , m_IndexBuffer{}
{
    m_ModelUBOs.clear();
    m_ModelUBOsMapped.clear();
}

void Model::Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& commandPool, const std::string& filePath)
{
    m_NrIndices = 0;
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};

    LoadModelFromFile(filePath, vertices, indices);
    InitDataBuffers(device, phyDevice, queue, commandPool, vertices, indices);
    InitUBOBuffers(device, phyDevice);
}

void Model::Destroy(VkDevice device)
{
    m_VertexBuffer.Destroy(device);
    m_IndexBuffer.Destroy(device);

    for (auto& ubo : m_ModelUBOs)
    {
        ubo.Destroy(device);
    }
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

void Model::SetRotation(glm::vec3 rotation)
{
    //glm::quat rot{ glm::rotation(glm::vec3(0.0f, 0.0f, 1.0f), rotation) };
}

void Model::SetScale(glm::vec3 scale)
{
    m_Transform.scale = scale;
    UpdateModelMatrix();
}

void Model::SetScale(float scale)
{
    SetScale(glm::vec3{ scale, scale, scale });
}

void Model::SetTranform(const Transform& transform)
{
    m_Transform = transform;
    UpdateModelMatrix();
}

const glm::mat4& Model::GetModelMatrix() const
{
    return m_ModelMatrix;
}

const std::vector<DataBuffer>& Model::GetBuffers() const
{
    return m_ModelUBOs;
}

void Model::UpdateUniformBuffers(uint32_t currentFrame)
{
    ModelUBO ubo{};
    ubo.model = m_ModelMatrix;

    memcpy(m_ModelUBOsMapped[currentFrame], &ubo, sizeof(ubo));
}

void Model::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet) const
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelUBO), &m_ModelMatrix);

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
    m_NrIndices = static_cast<uint32_t>(indices.size());
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

void Model::InitUBOBuffers(VkDevice device, VkPhysicalDevice phyDevice)
{
    VkDeviceSize bufferSize{ sizeof(ModelUBO) };

    m_ModelUBOs.resize(MAX_FRAMES_IN_FLIGHT);
    m_ModelUBOsMapped.resize(MAX_FRAMES_IN_FLIGHT);

    VkBufferUsageFlags uniformBuffersUsage{ VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT };
    VkMemoryPropertyFlags uniformBuffersProperties{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

    for (size_t frameIdx{}; frameIdx < MAX_FRAMES_IN_FLIGHT; frameIdx++)
    {
        m_ModelUBOs[frameIdx].Initialize(device, phyDevice, uniformBuffersProperties, bufferSize, uniformBuffersUsage);
        m_ModelUBOs[frameIdx].Map(device, bufferSize, &m_ModelUBOsMapped[frameIdx]);
        UpdateUniformBuffers(static_cast<uint32_t>(frameIdx));
    }
}

void Model::UpdateModelMatrix()
{
    m_ModelMatrix = m_Transform.GetModelMatrix();
}