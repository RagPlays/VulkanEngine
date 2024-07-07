#include <algorithm>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Camera.h"
#include "VulkanStructs.h"
#include "VulkanUtils.h"
#include "Timer.h"

Camera::Camera()
    : m_Position{ glm::vec3{ 0.f, 2.f, 5.f } }
    , m_Front{ glm::vec3{ 0.f, 0.f, 1.f } }
    , m_Up{ glm::vec3{ 0.f, 1.f, 0.f } }
    , m_Right{ glm::vec3{ 1.f, 0.f, 0.f } }
    , m_WorldUp{ glm::vec3{ 0.f, 1.f, 0.f } }
    , m_Yaw{ -90.f }
    , m_Pitch{ 0.f }
    //, m_Roll{ 0.f }
    , m_MovementSpeed{ 3.f }
    , m_Sensitivity{ 0.13f }
{
    UpdateCameraVectors();
}

void Camera::Initialize(VkDevice device, VkPhysicalDevice phyDevice, float aspectRatio, GLFWwindow* window)
{
    m_FOV = 45.f;
    m_Near = 0.1f;
    m_Far = 100.f;
    m_Window = window;
    m_AspectRatio = aspectRatio;

    UpdateCameraVectors();

    VkDeviceSize bufferSize{ sizeof(CameraUBO) };

    m_UniformBuffers.resize(g_MaxFramesInFlight);
    m_UniformBuffersMapped.resize(g_MaxFramesInFlight);

    VkBufferUsageFlags uniformBuffersUsage{ VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT };
    VkMemoryPropertyFlags uniformBuffersProperties{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

    for (size_t idx{}; idx < g_MaxFramesInFlight; idx++)
    {
        m_UniformBuffers[idx].Initialize(device, phyDevice, uniformBuffersProperties, bufferSize, uniformBuffersUsage);
        m_UniformBuffers[idx].Map(device, bufferSize, &m_UniformBuffersMapped[idx]);
        UpdateUniformBufferObjects(static_cast<uint32_t>(idx));
    }
}

void Camera::Destroy(VkDevice device)
{
    for (auto& ubo : m_UniformBuffers)
    {
        ubo.Destroy(device);
    }
}

void Camera::Update(uint32_t currentFrame)
{
    m_CurrentFrame = currentFrame;

    bool change{ false };
    const float deltaTime{ Timer::Get().GetElapsedSec() };
    float moveSpeed{ m_MovementSpeed };

    // KeyBoard Movement //
    if (glfwGetKey(m_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        moveSpeed *= 2.f;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
    {
        m_Position += m_Front * moveSpeed * deltaTime;
        change = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
    {
        m_Position -= m_Front * moveSpeed * deltaTime;
        change = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
    {
        m_Position -= m_Right * moveSpeed * deltaTime;
        change = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
    {
        m_Position += m_Right * moveSpeed * deltaTime;
        change = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        m_Position -= m_Up * moveSpeed * deltaTime;
        change = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS)
    {
        m_Position += m_Up * moveSpeed * deltaTime;
        change = true;
    }

    // Mouse Movement //
    double mouseX;
    double mouseY;
    glfwGetCursorPos(m_Window, &mouseX, &mouseY);

    if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) ||
        glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT))
    {
        m_Yaw += static_cast<float>(mouseX - m_LastMouseX) * m_Sensitivity;
        m_Pitch += static_cast<float>(m_LastMouseY - mouseY) * m_Sensitivity;
        m_Pitch = std::clamp(m_Pitch, -89.f, 89.f);
        change = true;
    }
    m_LastMouseX = mouseX;
    m_LastMouseY = mouseY;

    if (change)
    {
        UpdateCameraVectors();
    }
    UpdateUniformBufferObjects(currentFrame);
}

const std::vector<DataBuffer>& Camera::GetUniformBuffers() const
{
    return m_UniformBuffers;
}

void Camera::UpdateCameraVectors()
{
    // Set Correct Variables
    glm::vec3 newFront{};
    newFront.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    newFront.y = sin(glm::radians(m_Pitch));
    newFront.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

    m_Front = glm::normalize(newFront);
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));

    /*glm::mat4 rollMatrix{ glm::rotate(glm::mat4{ 1.f }, glm::radians(m_Roll), m_Front) };
    m_Right = glm::vec3{ rollMatrix * glm::vec4{ m_Right, 0.f } };
    m_Up = glm::vec3{ rollMatrix * glm::vec4{ m_Up, 0.0f } };*/

    m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_Near, m_Far);
    m_ProjectionMatrix[1][1] *= -1;
}

void Camera::UpdateUniformBufferObjects(uint32_t currentFrame)
{
    CameraUBO ubo{ m_ViewMatrix, m_ProjectionMatrix };

    memcpy(m_UniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}