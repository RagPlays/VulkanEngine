#version 450

layout(push_constant) uniform ModelUBO
{
    mat3 model;
} modelUBO;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main()
{
    vec3 transformedPosition = modelUBO.model * vec3(inPosition, 1.0);
    gl_Position = vec4(transformedPosition.xy, 0.0, 1.0);
    
    fragColor = inColor;
}