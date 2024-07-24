#version 450

layout(push_constant) uniform Model2DUBO
{
    mat4 model;
} modelUBO;

layout(binding = 0) uniform CameraUBO
{
    mat4 view;
    mat4 proj;
} cameraUBO;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = cameraUBO.proj * modelUBO.model * vec4(inPosition, -1.0, 1.0);
    fragColor = inColor;
}