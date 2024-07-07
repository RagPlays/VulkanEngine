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
    "Libraries/tiny_obj_loader.h"
    "main.cpp"
    "VulkanBase.h"
    "VulkanBase.cpp"
    "VulkanUtils.h"
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
    "Image.cpp"
    "ImageView.h"
    "ImageView.cpp"
    "Sampler.h"
    "Sampler.cpp"
    "Model.h"
    "Model.cpp"
    "Scene.h"
    "Scene.cpp"
    "Shader.h"
    "Shader.cpp"
    "Camera.h"
    "Camera.cpp"
    "Timer.h"
    "Timer.cpp"
    "Singleton.h"
    "SyncObjects.h"
    "SyncObjects.cpp"
    "VulkanInstance.h"
    "VulkanInstance.cpp"
    "Surface.h"
    "Surface.cpp"
    "Texture.h"
    "Texture.cpp"
    "Window.h"
    "Window.cpp"
    
     "GraphicsPipeline3D.h" "GraphicsPipeline3D.cpp")

set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/Project/Resources")
set(DESTINATION_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# Copy resources to build directory using add_custom_target with DEPENDS
file(COPY ${RESOURCES_DIR} DESTINATION ${DESTINATION_DIR})
add_custom_target(CopyResources ALL
    COMMENT "Copying resources directory to build directory"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${RESOURCES_DIR} ${DESTINATION_DIR}
    DEPENDS ${RESOURCES_DIR}
)
# Create the executable
add_executable(VulkanEngine ${SOURCES} ${GLSL_SOURCE_FILES} )
add_dependencies(${PROJECT_NAME} Shaders)

# Link libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(VulkanEngine PRIVATE ${Vulkan_LIBRARIES} glfw)