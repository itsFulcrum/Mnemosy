cmake_minimum_required(VERSION 3.1)

set(shadIncludeLib "shadinclude")

add_library(${shadIncludeLib} INTERFACE)
target_include_directories(
    ${shadIncludeLib}
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:glm>
)