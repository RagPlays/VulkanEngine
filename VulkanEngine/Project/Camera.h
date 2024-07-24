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

#include "DataBuffer.h"

class Camera
{
public:

    Camera();
    ~Camera() = default;

    void Initialize(VkDevice device, VkPhysicalDevice phyDevice, float aspectRatio, GLFWwindow* window);
    void Destroy(VkDevice device);

    void Update(uint32_t currentFrame);

    const std::vector<DataBuffer>& GetUniformBuffers() const;

private:

    void UpdateCameraVectors();
    void UpdateUniformBufferObjects(uint32_t currentFrame);

private:

    GLFWwindow* m_Window;
    float m_AspectRatio;

    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    float m_Yaw;
    float m_Pitch;
    //float m_Roll;

    double m_LastMouseX;
    double m_LastMouseY;

    float m_MovementSpeed;
    float m_Sensitivity;

    float m_FOV;
    float m_Near;
    float m_Far;

    glm::mat4 m_ViewMatrix{};
    glm::mat4 m_PerspectiveProjMatrix{};
    glm::mat4 m_OrthoProjMatrix{};

    uint32_t m_CurrentFrame;
    std::vector<DataBuffer> m_UniformBuffers;
    std::vector<void*> m_UniformBuffersMapped;

};

#endif // !CAMERA_H