#ifndef VULKANUTILS_H
#define VULKANUTILS_H

#include <vector>
#include <string>
#include <fstream>

#include <vulkan/vulkan.h>

const int g_MaxFramesInFlight{ 2 };

const std::string g_Model3DPath1{ "Resources/Models/viking_room.obj" };
const std::string g_Model3DPath2{ "Resources/Models/cube.obj" };

const std::string g_PlaneModel{ "Resources/Models/plane.obj" };
const std::string g_TriangleModel{ "Resources/Models/triangle.obj" };

const std::string g_TexturePath1{ "Resources/Textures/viking_room.png" };
const std::string g_TexturePath2{ "Resources/texture.jpg" };

#endif //!VULKANUTILS_H