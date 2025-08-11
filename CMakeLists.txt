cmake_minimum_required(VERSION 3.10.0)

project(Aether-GameEngine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCE_FILES
    "src/core/window_context.cpp"
    "src/core/rendering/vulkan/vulkan_context.cpp"
    "src/core/rendering/vulkan/vulkan_device_context.cpp"
    "src/core/rendering/vulkan/vulkan_swapchain_context.cpp"
    "src/core/rendering/vulkan/renderer.cpp"
    "src/main.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

