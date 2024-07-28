# Set minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Project Name
project(VulkanEngine LANGUAGES CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the required packages
find_package(Vulkan REQUIRED)

# Include Directories
include_directories(${Vulkan_INCLUDE_DIRS})

# Add subdirectory for your project
add_subdirectory(Project)

# Fetch glfw with git
include(FetchContent)

# Declare GLFW and disable tests and examples
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8 # Specify the version you need
)
FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
  FetchContent_Populate(glfw)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
  add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Declare the nlohmann/json library as a fetchable content
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3 # Specify the version of the library you want to use
)

# Make the fetched content available
FetchContent_MakeAvailable(json)

# If using validation layers, copy the required JSON files (optional)
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#    "${CMAKE_SOURCE_DIR}/path_to_json/VK_LAYER_KHRONOS_validation.json"
#    $<TARGET_FILE_DIR:${PROJECT_NAME}>)