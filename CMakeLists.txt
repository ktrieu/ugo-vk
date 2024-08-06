cmake_minimum_required(VERSION 3.20...3.30)

set (CMAKE_CXX_STANDARD 20)

project(
    ugo-vk
    VERSION 0.1
    LANGUAGES CXX
)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/target/")

find_package(glfw3 CONFIG REQUIRED)

find_package(Vulkan REQUIRED)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

find_package(fmt CONFIG REQUIRED)

set(SPIRV_FILES)

function(compile_shader shader_file)
    cmake_path(GET shader_file EXTENSION extension)
    cmake_path(GET shader_file STEM filename)
    if (MSVC)
        set(output_file ${EXECUTABLE_OUTPUT_PATH}$<CONFIGURATION>/shader/${filename}${extension}.spv)
    else()
        set(output_file ${EXECUTABLE_OUTPUT_PATH}shader/${filename}${extension}.spv)
    endif()
    add_custom_command(
        OUTPUT ${output_file}
        COMMAND ${glslc_executable} ${shader_file} -o ${output_file}
        DEPENDS ${shader_file}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    list(APPEND SPIRV_FILES ${output_file})
    set(SPIRV_FILES ${SPIRV_FILES} PARENT_SCOPE)
endfunction()

set(SHADERS
    src/shader/tri.vert
    src/shader/tri.frag
)

foreach(shader_file ${SHADERS})
    compile_shader(${shader_file})
endforeach()

add_executable(ugo-vk-bin 
    "src/main.cpp"
    "src/window/window.h"
    "src/window/window.cpp"
    "src/logger.h"
    "src/logger.cpp"
    "src/vk/context.h"
    "src/vk/context.cpp"
    "src/vk/device.h"
    "src/vk/device.cpp"
    "src/vk/vulkan_error.h"
    "src/vk/vulkan_error.cpp"
    "src/vk/physical_device.h"
    "src/vk/physical_device.cpp"
    "src/vk/swapchain.h"
    "src/vk/swapchain.cpp"
    "src/vk/pipeline_builder.h"
    "src/vk/pipeline_builder.cpp"
    "src/vk/sync.h"
    "src/vk/sync.cpp"
    "src/vk/command_buffer.h"
    "src/vk/command_buffer.cpp"
)

target_include_directories(ugo-vk-bin PRIVATE src)

target_link_libraries(ugo-vk-bin glfw)
target_link_libraries(ugo-vk-bin Vulkan::Vulkan)
target_link_libraries(ugo-vk-bin fmt::fmt)

add_custom_target(compile_shaders ALL DEPENDS ${SPIRV_FILES})
add_dependencies(ugo-vk-bin compile_shaders)
target_sources(ugo-vk-bin PRIVATE ${SPIRV_FILES})