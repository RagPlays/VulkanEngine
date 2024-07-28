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
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} -g ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

set(STB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Libs/stb")
set(OBJLOADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Libs/tinyobjloader")

set(SOURCES 
    "main.cpp"
    "Application.h"
    "Application.cpp"
    "VulkanUtils.h"
    "VulkanStructs.h"
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
    "GraphicsPipeline3D.h"
    "GraphicsPipeline3D.cpp"
    "GraphicsPipeline2D.h"
    "GraphicsPipeline2D.cpp"
    "Vertex.h"
    "DepthBuffer.cpp"
    "DepthBuffer.h"
    "Swapchain.cpp"
    "Swapchain.h"
    "GraphicsPipeline3DIR.h"
    "GraphicsPipeline3DIR.cpp"
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${GLSL_SOURCE_FILES} )
add_dependencies(${PROJECT_NAME} Shaders)

# Link libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# Include STB image
target_include_directories(${PROJECT_NAME} PRIVATE "${STB_DIR}/include")
# Include tinyObjLoader
target_include_directories(${PROJECT_NAME} PRIVATE "${OBJLOADER_DIR}/include")

# Copy resources
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/Project/Resources")
file(COPY ${RESOURCES_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(CopyResources ALL
    COMMENT "Copying resources directory to build directory"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${RESOURCES_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${RESOURCES_DIR}
)

# Visual Leak Detector
if(WIN32 AND CMAKE_BUILD_TYPE MATCHES Debug)

    add_compile_definitions(ENABLE_VLD=1)
    set(VLD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Libs/vld")
    add_library(vld STATIC IMPORTED)

    set_target_properties(vld PROPERTIES
        IMPORTED_LOCATION "${VLD_DIR}/lib/vld.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${VLD_DIR}/include/"
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} vld)

    set(DLL_SOURCE_DIR "${VLD_DIR}/lib")

    file(GLOB_RECURSE DLL_FILES
        "${DLL_SOURCE_DIR}/*.dll"
        "${DLL_SOURCE_DIR}/*.manifest"
    )

    foreach(DLL ${DLL_FILES})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${DLL}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    endforeach(DLL)

endif()