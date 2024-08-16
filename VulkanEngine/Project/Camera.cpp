#include <algorithm>
#include <iostream>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Camera.h"
#include "VulkanStructs.h"
#include "VulkanUtils.h"
#include "Timer.h"
#include "Window.h"
#include "VulkanInstance.h"

Camera::Camera()
    : m_Position{ glm::vec3{ 0.f, 2.f, 5.f } }
    , m_Front{ glm::vec3{ 0.f, 0.f, 1.f } }
    , m_Up{ glm::vec3{ 0.f, 1.f, 0.f } }
    , m_Right{ glm::vec3{ 1.f, 0.f, 0.f } }
    , m_WorldUp{ glm::vec3{ 0.f, 1.f, 0.f } }
    , m_Rotation{ 0.f, -90.f, 0.f }
    , m_MovementSpeed{ 3.f }
    , m_Sensitivity{ 0.13f }
{
    UpdateCameraVectors();
}

void Camera::Initialize(const VulkanInstance& instance, const Window& window)
{
    const VkDevice& device{ instance.GetVkDevice() };
    const VkPhysicalDevice& phyDevice{ instance.GetVkPhysicalDevice() };
    const float aspectRatio{ window.GetAspectRatio() };

    m_FOV = 45.f;
    m_Near = 0.1f;
    m_Far = 100.f;
    m_Window = window.GetWindow();
    m_AspectRatio = aspectRatio;

    UpdateCameraVectors();

    VkDeviceSize bufferSize{ sizeof(CameraUBO) };

    m_UniformBuffers.resize(g_MaxFramesInFlight);

    VkBufferUsageFlags uniformBuffersUsage{ VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT };
    VkMemoryPropertyFlags uniformBuffersProperties
    {
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    for (size_t idx{}; idx < g_MaxFramesInFlight; idx++)
    {
        m_UniformBuffers[idx].Initialize(device, phyDevice, uniformBuffersProperties, bufferSize, uniformBuffersUsage);
        UpdateUniformBufferObjects(device, static_cast<uint32_t>(idx));
    }
}

void Camera::Destroy(VkDevice device)
{
    for (auto& ubo : m_UniformBuffers)
    {
        ubo.Destroy(device);
    }
}

void Camera::Update(VkDevice device, uint32_t currentFrame)
{
    m_InputState.keyChange = false;
    m_InputState.mouseChange = false;

    const float deltaTime{ Timer::Get().GetElapsedSec() };
    float moveSpeed{ m_MovementSpeed };

    // KeyBoard Movement //
    if (glfwGetKey(m_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        moveSpeed *= 3.5f;
        m_InputState.keyChange = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
    {
        m_Position += m_Front * moveSpeed * deltaTime;
        m_InputState.keyChange = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
    {
        m_Position -= m_Front * moveSpeed * deltaTime;
        m_InputState.keyChange = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
    {
        m_Position -= m_Right * moveSpeed * deltaTime;
        m_InputState.keyChange = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
    {
        m_Position += m_Right * moveSpeed * deltaTime;
        m_InputState.keyChange = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        m_Position -= m_WorldUp * moveSpeed * deltaTime;
        m_InputState.keyChange = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS)
    {
        m_Position += m_WorldUp * moveSpeed * deltaTime;
        m_InputState.keyChange = true;
    }

    // Mouse Movement //
    double mouseX;
    double mouseY;
    glfwGetCursorPos(m_Window, &mouseX, &mouseY);

    if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) ||
        glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT))
    {
        m_Rotation.y += static_cast<float>(mouseX - m_InputState.lastMouseX) * m_Sensitivity;
        m_Rotation.x += static_cast<float>(m_InputState.lastMouseY - mouseY) * m_Sensitivity;
        m_Rotation.x = std::clamp(m_Rotation.x, -89.f, 89.f);
        m_InputState.mouseChange = true;
    }
    m_InputState.lastMouseX = mouseX;
    m_InputState.lastMouseY = mouseY;

    if (m_InputState.keyChange || m_InputState.mouseChange)
    {
        UpdateCameraVectors();
    }
    UpdateUniformBufferObjects(device, currentFrame);
}

const std::vector<DataBuffer>& Camera::GetUniformBuffers() const
{
    return m_UniformBuffers;
}

const glm::vec3& Camera::GetDirection() const
{
    return m_Front;
}

void Camera::UpdateCameraVectors()
{
    // Set Correct Variables
    const glm::vec3 newFront
    {
        cos(glm::radians(m_Rotation.y))* cos(glm::radians(m_Rotation.x)),
        sin(glm::radians(m_Rotation.x)),
        sin(glm::radians(m_Rotation.y))* cos(glm::radians(m_Rotation.x))
    };

    m_Front = glm::normalize(newFront);
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));

    // when roll is needed
    /*glm::mat4 rollMatrix{ glm::rotate(glm::mat4{ 1.f }, glm::radians(m_Rotation.z), m_Front) };
    m_Right = glm::vec3{ rollMatrix * glm::vec4{ m_Right, 0.f } };
    m_Up = glm::vec3{ rollMatrix * glm::vec4{ m_Up, 0.0f } };*/

    m_CameraMatrix.view = glm::lookAt(m_Position, m_Position + m_Front, m_Up);

    m_CameraMatrix.proj = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_Near, m_Far);
    m_CameraMatrix.proj[1][1] *= -1;
}

void Camera::UpdateUniformBufferObjects(VkDevice device, uint32_t currentFrame)
{
    const VkDeviceSize& bufferSize{ m_UniformBuffers[currentFrame].GetSizeInBytes() };
    m_UniformBuffers[currentFrame].Upload(device, bufferSize, &m_CameraMatrix);
}