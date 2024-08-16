#if defined DEBUG || defined _DEBUG
#include "vld.h"
#endif // DEBUG

#include "Application.h"

/// !!!!!!!!
// Lot of Vulkan Code based on https://vulkan-tutorial.com/ //
// /////

int main()
{
	Application vulkanApplication{};
    try
    {
        vulkanApplication.Run();
    }
    catch (const std::exception& execption)
    {
        std::cerr << execption.what() << "\n";
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Error!!\n";
    }
    return EXIT_SUCCESS;
}