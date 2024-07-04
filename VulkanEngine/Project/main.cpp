#include "VulkanBase.h"

/// !!!!!!!!
// Vulkan Code based on https://vulkan-tutorial.com/
// /////

int main()
{
	VulkanBase vulkanApplication{};

    try
    {
        vulkanApplication.Run();
    }
    catch (const std::exception& execption)
    {
        std::cerr << execption.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}