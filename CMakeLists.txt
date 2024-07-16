cmake_minimum_required(VERSION 3.19...3.30)

set (CMAKE_CXX_STANDARD 20)

project(
    ugo-vk
    VERSION 0.1
    LANGUAGES CXX
)

set(EXECUTABLE_OUTPUT_PATH "target/")

add_executable(ugo-vk-bin src/main.cpp)