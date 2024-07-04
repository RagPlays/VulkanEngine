# Source files
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Shaders")

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

set(SOURCES
    "Libraries/stb_image.h"
    "main.cpp"
    "VulkanBase.h"
    "VulkanBase.cpp"
    "VulkanUtils.h"
    "VulkanUtils.cpp"
    "VulkanStructs.h"
    "VulkanStructs.cpp"
    "RenderPass.h"
    "RenderPass.cpp"
    "CommandPool.h"
    "CommandPool.cpp"
    "CommandBuffer.h" 
    "CommandBuffer.cpp"
    "DataBuffer.h"
    "DataBuffer.cpp"
    "Image.h"
    "Image.cpp" "ImageView.h" "ImageView.cpp" "Sampler.h" "Sampler.cpp")

set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/Project/Resources")
file(COPY ${RESOURCES_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(CopyResources ALL
    COMMENT "Copying resources directory to build directory"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${RESOURCES_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${RESOURCES_DIR}
)

# Create the executable
add_executable(VulkanEngine ${SOURCES} ${GLSL_SOURCE_FILES} )
add_dependencies(${PROJECT_NAME} Shaders)

# Link libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(VulkanEngine PRIVATE ${Vulkan_LIBRARIES} glfw)