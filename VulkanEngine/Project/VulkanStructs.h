#ifndef VULKANSTRUCTS_H
#define VULKANSTRUCTS_H

#include <optional>
#include <vector>
#include <array>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily{};
	std::optional<uint32_t> presentFamily{};

	bool IsComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities{};
	std::vector<VkSurfaceFormatKHR> formats{};
	std::vector<VkPresentModeKHR> presentModes{};
};

struct CameraUBO
{
	alignas(16) glm::mat4 view{};
	alignas(16) glm::mat4 proj{};
};

struct Model3DUBO
{
	alignas(16) glm::mat4 model{};
};

struct Model2DUBO
{
	alignas(16) glm::mat3 model{};
};

struct Transform2D
{
	glm::vec2 position{ 0.f, 0.f };
	float rotation{ 0.f }; // Rotation in radians around Z axis
	glm::vec2 scale{ 1.f, 1.f };

	glm::mat3 GetModelMatrix() const
	{
		const float cosTheta{ std::cos(rotation) };
		const float sinTheta{ std::sin(rotation) };

		glm::mat3 translateMatrix
		{
			{1.f, 0.f, position.x},
			{0.f, 1.f, position.y},
			{0.f, 0.f, 1.f}
		};

		glm::mat3 rotateMatrix
		{
			{cosTheta, -sinTheta, 0.f},
			{sinTheta, cosTheta, 0.f},
			{0.f, 0.f, 1.f}
		};

		glm::mat3 scaleMatrix
		{
			{scale.x, 0.f, 0.f},
			{0.f, scale.y, 0.f},
			{0.f, 0.f, 1.f}
		};
		return scaleMatrix * rotateMatrix * translateMatrix;
	}
};

struct Transform3D
{
	glm::vec3 position{ 0.f };
	glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
	glm::vec3 scale{ 1.f, 1.f, 1.f };

	glm::mat4 GetModelMatrix() const
	{
		glm::mat4 model{ glm::mat4{ 1.f } };
		model = glm::translate(model, position);
		model = model * glm::mat4_cast(rotation);
		model = glm::scale(model, scale);
		return model;
	}
};

#endif // !VULKANSTRUCTS_H