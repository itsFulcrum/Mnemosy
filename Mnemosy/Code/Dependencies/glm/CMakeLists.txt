cmake_minimum_required(VERSION 3.1)

set(glmLib "glm")

add_library(${glmLib} INTERFACE)
target_include_directories(
    ${glmLib}
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:glm>
)