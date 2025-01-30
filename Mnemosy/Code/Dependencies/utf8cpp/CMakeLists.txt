cmake_minimum_required(VERSION 3.21)

set(utf_8cpp "utf8cpp")

add_library(${utf_8cpp} INTERFACE)
target_include_directories(
    ${utf_8cpp}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)