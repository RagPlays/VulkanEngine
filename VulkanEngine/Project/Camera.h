#ifndef CAMERA_H
#define CAMERA_H

#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VulkanStructs.h"
#include "DataBuffer.h"

class VulkanInstance;
class Window;

class Camera
{
public:

    Camera();
    ~Camera() = default;

    void Initialize(const VulkanInstance& instance, const Window& window);
    void Destroy(VkDevice device);

    void Update(VkDevice device, uint32_t currentFrame);

    const std::vector<DataBuffer>& GetUniformBuffers() const;
    const glm::vec3& GetDirection() const;

private:

    void UpdateCameraVectors();
    void UpdateUniformBufferObjects(VkDevice device, uint32_t currentFrame);

private:

    GLFWwindow* m_Window;
    float m_AspectRatio;

    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    glm::vec3 m_Rotation; // (Pitch, Yaw, Roll)

    InputState m_InputState;

    float m_MovementSpeed;
    float m_Sensitivity;

    float m_FOV;
    float m_Near;
    float m_Far;

    CameraUBO m_CameraMatrix;

    std::vector<DataBuffer> m_UniformBuffers;

};

#endif // !CAMERA_H