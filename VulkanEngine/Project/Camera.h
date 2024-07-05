#ifndef CAMERA_H
#define CAMERA_H

#include <vector>

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
    float m_Roll;

    double m_LastMouseX;
    double m_LastMouseY;

    float m_MovementSpeed;
    float m_Sensitivity;

    float m_FOV;
    float m_Near;
    float m_Far;

    std::vector<DataBuffer> m_UniformBuffers;
    std::vector<void*> m_UniformBuffersMapped;

};

#endif // !CAMERA_H