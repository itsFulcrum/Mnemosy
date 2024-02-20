cmake_minimum_required(VERSION 3.1)

set(stbImageLib "stbImage")

add_library(${stbImageLib} INTERFACE)
target_include_directories(
    ${stbImageLib}
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)