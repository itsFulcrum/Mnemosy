cmake_minimum_required(VERSION 3.21)

set(stbImage_ReadLib "stbImageRead")

add_library(${stbImage_ReadLib} INTERFACE)
target_include_directories(
    ${stbImage_ReadLib}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
