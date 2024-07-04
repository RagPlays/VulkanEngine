#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:

    Camera();
    ~Camera() = default;

    void Update(float deltaTime, GLFWwindow* window);

    // Getters for view and projection matrices
    glm::mat4 Camera::GetModelMatrix() const;
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspectRatio) const;

private:

    void LookAt(const glm::vec3 target);

private:

    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    float m_Yaw;
    float m_Pitch;

    double m_LastMouseX;
    double m_LastMouseY;

    float m_MovementSpeed;
    float m_Sensitivity;

    void UpdateCameraVectors();
};

#endif // !CAMERA_H