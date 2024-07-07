#ifndef VULKANUTILS_H
#define VULKANUTILS_H

#include <vector>
#include <string>
#include <fstream>

#include <vulkan/vulkan.h>

const int g_MaxFramesInFlight{ 1 };

const std::string g_Model1Path{ "Resources/Models/viking_room.obj" };
const std::string g_Texture1Path{ "Resources/Models/viking_room.png" };

const std::string g_Model2Path{ "Resources/Models/cube.obj" };
const std::string g_Texture2Path{ "Resources/texture.jpg" };

const std::string g_Model3Path{ "Resources/Models/plane.obj" };
//const std::string g_Texture3Path{ "Resources/Models/viking_room.png" };

#endif //!VULKANUTILS_H