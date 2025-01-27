#ifndef VULKANUTILS_H
#define VULKANUTILS_H

#include <vector>
#include <string>
#include <fstream>

#include <vulkan/vulkan.h>

const int g_MaxFramesInFlight{ 2 };

constexpr const char* g_Model3DPath1{ "Resources/Models/viking_room.obj" };
constexpr const char* g_CubeModel{ "Resources/Models/cube.obj" };

constexpr const char* g_PlaneModel{ "Resources/Models/plane.obj" };
constexpr const char* g_TriangleModel{ "Resources/Models/triangle.obj" };

constexpr const char* g_TexturePath1{ "Resources/Textures/viking_room.png" };
constexpr const char* g_TexturePath2{ "Resources/Textures/texture.jpg" };
constexpr const char* g_TexturePath3{ "Resources/Textures/stone_cube_texture.jpeg" };

#endif //!VULKANUTILS_H