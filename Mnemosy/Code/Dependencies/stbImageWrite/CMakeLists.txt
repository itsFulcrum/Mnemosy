cmake_minimum_required(VERSION 3.21)

set(stbImage_WriteLib "stbImageWrite")

add_library(${stbImage_WriteLib} INTERFACE)
target_include_directories(
    ${stbImage_WriteLib}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
