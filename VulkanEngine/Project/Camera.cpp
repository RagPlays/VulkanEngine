#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Camera.h"

Camera::Camera()
    : m_Position{ glm::vec3{ 0.f, 0.f, 5.f } }
    , m_Front{ glm::vec3{ 0.f, 0.f, 1.f } }
    , m_Up{ glm::vec3{ 0.f, 1.f, 0.f } }
    , m_Right{ glm::vec3{ 1.f, 0.f, 0.f } }
    , m_WorldUp{ glm::vec3{ 0.f, 1.f, 0.f } }
    , m_Yaw{ -90.f }
    , m_Pitch{ 0.f }
    , m_MovementSpeed{ 2.f }
    , m_Sensitivity{ 0.13f }
{
    UpdateCameraVectors();
}

void Camera::Update(float deltaTime, GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        m_Position += m_Front * m_MovementSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        m_Position -= m_Front * m_MovementSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        m_Position -= m_Right * m_MovementSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        m_Position += m_Right * m_MovementSpeed * deltaTime;
    }

    double mouseX;
    double mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
    {
        m_Yaw += static_cast<float>(mouseX - m_LastMouseX) * m_Sensitivity;
        m_Pitch += static_cast<float>(m_LastMouseY - mouseY) * m_Sensitivity; // reversed since y-coordinates range from bottom to top
    }
    m_LastMouseX = mouseX;
    m_LastMouseY = mouseY;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (m_Pitch > 89.0f) m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = -89.0f;

    UpdateCameraVectors();
}

glm::mat4 Camera::GetModelMatrix() const
{
    return glm::mat4{ 1.f };
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) const
{
    return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
}

void Camera::LookAt(const glm::vec3 target)
{
    glm::vec3 direction{ glm::normalize(target - m_Position) };
    m_Yaw = glm::degrees(atan2(direction.z, direction.x)) - 90.0f;
    m_Pitch = glm::degrees(asin(direction.y));

    UpdateCameraVectors();
}

void Camera::UpdateCameraVectors()
{
    glm::vec3 newFront{};
    newFront.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    newFront.y = sin(glm::radians(m_Pitch));
    newFront.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(newFront);

    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}