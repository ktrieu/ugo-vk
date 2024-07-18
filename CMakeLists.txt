cmake_minimum_required(VERSION 3.19...3.30)

set (CMAKE_CXX_STANDARD 20)

project(
    ugo-vk
    VERSION 0.1
    LANGUAGES CXX
)

set(EXECUTABLE_OUTPUT_PATH "target/")

find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(ugo-vk-bin 
    src/main.cpp
    src/window/window.h
    src/window/window.cpp
    src/vk/vulkan_context.h
    src/vk/vulkan_context.cpp
)

target_link_libraries(ugo-vk-bin glfw)
target_link_libraries(ugo-vk-bin Vulkan::Vulkan)