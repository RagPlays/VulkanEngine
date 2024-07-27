#include <array>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct Vertex2D
{
	glm::vec2 pos{};
	glm::vec3 color{};

	bool operator==(const Vertex2D& other) const
	{
		return pos == other.pos && color == other.color;
	}

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex2D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex2D, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex2D, color);
		return attributeDescriptions;
	}
};

namespace std
{
	template<>
	struct hash<Vertex2D>
	{
		std::size_t operator()(const Vertex2D& vertex) const
		{
			std::size_t h1 = std::hash<float>()(vertex.pos.x) ^ (std::hash<float>()(vertex.pos.y) << 1);
			std::size_t h2 = std::hash<float>()(vertex.color.r) ^ (std::hash<float>()(vertex.color.g) << 1) ^ (std::hash<float>()(vertex.color.b) << 2);
			return h1 ^ (h2 << 1);
		}
	};
}

struct Vertex3D
{
	glm::vec3 pos{};
	glm::vec3 color{};
	glm::vec2 texCoord{};

	bool operator==(const Vertex3D& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex3D);
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

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex3D, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3D, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex3D, texCoord);

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

namespace std
{
	template<>
	struct hash<Vertex3D>
	{
		size_t operator()(Vertex3D const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct Vertex3DIR
{
	glm::vec3 pos{};
	glm::vec3 color{};

	bool operator==(const Vertex3DIR& other) const
	{
		return pos == other.pos && color == other.color;
	}

	static std::array<VkVertexInputBindingDescription, 2> GetBindingDescriptions()
	{
		std::array<VkVertexInputBindingDescription, 2> bindingDescriptions{};

		// Vertex binding description
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex3DIR);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		// Instance binding description
		bindingDescriptions[1].binding = 1;
		bindingDescriptions[1].stride = sizeof(ModelUBO);
		bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

		return bindingDescriptions;
	}

	static std::array<VkVertexInputAttributeDescription, 6> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions{};

		// Vertex attributes
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex3DIR, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3DIR, color);

		// Instance attributes (model matrix as 4 vec4s)
		attributeDescriptions[2].binding = 1;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(ModelUBO, model) + 0 * sizeof(glm::vec4);

		attributeDescriptions[3].binding = 1;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(ModelUBO, model) + 1 * sizeof(glm::vec4);

		attributeDescriptions[4].binding = 1;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[4].offset = offsetof(ModelUBO, model) + 2 * sizeof(glm::vec4);

		attributeDescriptions[5].binding = 1;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[5].offset = offsetof(ModelUBO, model) + 3 * sizeof(glm::vec4);

		return attributeDescriptions;
	}
};

namespace std
{
	template<>
	struct hash<Vertex3DIR>
	{
		size_t operator()(Vertex3DIR const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1);
		}
	};
}