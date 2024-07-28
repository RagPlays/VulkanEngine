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
};

struct Vertex3D
{
	glm::vec3 pos{};
	glm::vec3 color{};
	glm::vec2 texCoord{};

	bool operator==(const Vertex3D& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

struct Vertex3DIR
{
	glm::vec3 pos{};
	glm::vec3 color{};
	glm::vec2 texCoord{};

	bool operator==(const Vertex3DIR& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
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

namespace Descriptions
{
	// BindingDescriptions //
	/*
		* VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
		* VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
	*/
	// AttributesDiscripions //
	/* FORMATS:
		* float: VK_FORMAT_R32_SFLOAT
		* vec2: VK_FORMAT_R32G32_SFLOAT
		* vec3: VK_FORMAT_R32G32B32_SFLOAT
		* vec4: VK_FORMAT_R32G32B32A32_SFLOAT
		* ivec2: VK_FORMAT_R32G32_SINT, a 2-component vector of 32-bit signed integers
		* uvec4: VK_FORMAT_R32G32B32A32_UINT, a 4-component vector of 32-bit unsigned integers
		* double: VK_FORMAT_R64_SFLOAT, a double-precision (64-bit) float
	*/

	// 2D //
	using BindingDescriptions2D = std::array<VkVertexInputBindingDescription, 1>;
	static BindingDescriptions2D Get2DBindingDescriptions()
	{
		BindingDescriptions2D bindingDescription{};

		bindingDescription[0].binding = 0;
		bindingDescription[0].stride = sizeof(Vertex2D);
		bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	using AttributeDescriptions2D = std::array<VkVertexInputAttributeDescription, 2>;
	static AttributeDescriptions2D Get2DAttributeDescriptions()
	{
		AttributeDescriptions2D attributeDescriptions{};

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

	// 3D //
	using BindingDescriptions3D = std::array<VkVertexInputBindingDescription, 1>;
	static BindingDescriptions3D Get3DBindingDescriptions()
	{
		BindingDescriptions3D bindingDescription{};

		bindingDescription[0].binding = 0;
		bindingDescription[0].stride = sizeof(Vertex3D);
		bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	using AttributeDescriptions3D = std::array<VkVertexInputAttributeDescription, 3>;
	static AttributeDescriptions3D Get3DAttributeDescriptions()
	{
		AttributeDescriptions3D attributeDescriptions{};

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

		return attributeDescriptions;
	}

	// 3DIR //
	using BindingDescriptions3DIR = std::array<VkVertexInputBindingDescription, 2>;
	static BindingDescriptions3DIR Get3DIRBindingDescriptions()
	{
		BindingDescriptions3DIR bindingDescriptions{};

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

	using AttributeDescriptions3DIR = std::array<VkVertexInputAttributeDescription, 7>;
	static AttributeDescriptions3DIR Get3DIRAttributeDescriptions()
	{
		AttributeDescriptions3DIR attributeDescriptions{};

		// Vertex attributes
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex3DIR, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3DIR, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex3DIR, texCoord);

		// Instance attributes (model matrix as 4 vec4s)
		for (size_t matrixVectorIdx{}; matrixVectorIdx < 4; ++matrixVectorIdx)
		{
			const size_t index{ 3 + matrixVectorIdx };
			attributeDescriptions[index].binding = 1;
			attributeDescriptions[index].location = static_cast<uint32_t>(index);
			attributeDescriptions[index].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[index].offset = static_cast<uint32_t>(offsetof(ModelUBO, model) + matrixVectorIdx * sizeof(glm::vec4));
		}

		return attributeDescriptions;
	}
}