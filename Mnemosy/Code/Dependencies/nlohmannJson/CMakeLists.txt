cmake_minimum_required(VERSION 3.21)


add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)