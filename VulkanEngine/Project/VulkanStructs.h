#ifndef VULKANSTRUCTS_H
#define VULKANSTRUCTS_H

#include <optional>
#include <vector>
#include <array>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily{};
	std::optional<uint32_t> presentFamily{};

	bool IsComplete();
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities{};
	std::vector<VkSurfaceFormatKHR> formats{};
	std::vector<VkPresentModeKHR> presentModes{};
};

struct UniformBufferObject
{
	alignas(16) glm::mat4 model{};
	alignas(16) glm::mat4 view{};
	alignas(16) glm::mat4 proj{};
};

struct Vertex
{
	glm::vec3 pos{};
	glm::vec3 color{};
	glm::vec2 texCoord{};

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		/*
		* VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
		* VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
		*/

		/*std::cout << "\nBinding Description:\n";
		std::cout << "\tbinding: " << bindingDescription.binding << "\n";
		std::cout << "\tstride: " << bindingDescription.stride << "\n";
		std::cout << "\tinputRate: " << bindingDescription.inputRate << "\n";*/

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		/* FORMATS:
		* float: VK_FORMAT_R32_SFLOAT
		* vec2: VK_FORMAT_R32G32_SFLOAT
		* vec3: VK_FORMAT_R32G32B32_SFLOAT
		* vec4: VK_FORMAT_R32G32B32A32_SFLOAT
		* ivec2: VK_FORMAT_R32G32_SINT, a 2-component vector of 32-bit signed integers
		* uvec4: VK_FORMAT_R32G32B32A32_UINT, a 4-component vector of 32-bit unsigned integers
		* double: VK_FORMAT_R64_SFLOAT, a double-precision (64-bit) float
		*/

		/*std::cout << "Attribute Descriptions:\n";
		for (const auto& attributeDescription : attributeDescriptions)
		{
			std::cout << "\tbinding: " << attributeDescription.binding << "\n";
			std::cout << "\tlocation: " << attributeDescription.location << "\n";
			std::cout << "\tformat: " << attributeDescription.format << "\n";
			std::cout << "\toffset: " << attributeDescription.offset << "\n\n";
		}*/

		return attributeDescriptions;
	}
};

#endif // !VULKANSTRUCTS_H