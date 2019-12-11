cmake_minimum_required(VERSION 3.11)
project(cosmic-void-viewer)

set (CMAKE_CXX_STANDARD 11)

## path to glfw3
add_subdirectory("${CMAKE_SOURCE_DIR}/externals/glfw-3.3")

include_directories("${CMAKE_SOURCE_DIR}/externals/glfw-3.3/include")

## path to gl3w
include_directories("${CMAKE_SOURCE_DIR}/externals/gl3w/include")

add_library(gl3w ${CMAKE_SOURCE_DIR}/externals/gl3w/src/gl3w.c)

set_target_properties(gl3w PROPERTIES COMPILE_FLAGS "-w")
target_include_directories(gl3w PUBLIC "$<BUILD_INTERFACE:"
                                       "externals/gl3w/include;"
                                       ">")

# find OpenGL
find_package(OpenGL REQUIRED)

## find ospcommon
find_package(ospcommon REQUIRED)
## find ospray
find_package(ospray REQUIRED)

# build imgui
add_subdirectory("${CMAKE_SOURCE_DIR}/externals/imgui")
include_directories("${CMAKE_SOURCE_DIR}/externals/imgui")

#build utils
add_subdirectory("${CMAKE_SOURCE_DIR}/utils")
include_directories("${CMAKE_SOURCE_DIR}/utils")

add_executable(viewer main.cpp)

# add_library(common dataLoader.cpp)

set_target_properties(viewer PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON)

target_link_libraries(viewer glfw
                             gl3w
                             imgui
                             util
                            #  transferFunction
                             ospcommon::ospcommon 
                             ospray::ospray)