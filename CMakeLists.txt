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
find_package(fmt CONFIG REQUIRED)

add_executable(ugo-vk-bin 
    src/main.cpp
    src/window/window.h
    src/window/window.cpp
    src/logger.h
    src/logger.cpp
    src/vk/vulkan_context.h
    src/vk/vulkan_context.cpp
    src/vk/vulkan_device.h
    src/vk/vulkan_device.cpp
    src/vk/vulkan_error.h
    src/vk/vulkan_error.cpp
    src/vk/physical_device.h
    src/vk/physical_device.cpp
)

target_link_libraries(ugo-vk-bin glfw)
target_link_libraries(ugo-vk-bin Vulkan::Vulkan)
target_link_libraries(ugo-vk-bin fmt::fmt)