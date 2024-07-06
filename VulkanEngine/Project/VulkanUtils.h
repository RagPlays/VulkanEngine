#ifndef VULKANUTILS_H
#define VULKANUTILS_H

#include <vector>
#include <string>
#include <fstream>

#include <vulkan/vulkan.h>

const int MAX_FRAMES_IN_FLIGHT{ 2 };

// 720p resolution (AR 16:9)
constexpr uint32_t g_WindowWidth{ 1280 };
constexpr uint32_t g_WindowHeight{ 720 };
constexpr float g_AspectRatio{ g_WindowWidth / static_cast<float>(g_WindowHeight) };

const std::string g_Model1Path{ "Resources/Models/viking_room.obj" };
const std::string g_Texture1Path{ "Resources/Models/viking_room.png" };

const std::string g_Model2Path{ "Resources/Models/cube.obj" };
const std::string g_Texture2Path{ "Resources/Models/viking_room.png" };

const std::string g_Model3Path{ "Resources/Models/plane.obj" };
const std::string g_Texture3Path{ "Resources/Models/viking_room.png" };

#endif //!VULKANUTILS_H