cmake_minimum_required(VERSION 3.20)
project(vulkan-add LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_library(vulkan_device_selector SHARED device_selector.cpp device_selector_capi.cpp)
target_link_libraries(vulkan_device_selector Vulkan::Vulkan)

add_executable(vulkan_learn main.cpp device_selector.cpp)
target_link_libraries(vulkan_learn Vulkan::Vulkan)