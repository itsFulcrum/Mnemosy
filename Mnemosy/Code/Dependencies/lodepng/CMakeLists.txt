cmake_minimum_required(VERSION 3.25)


project(lodepng)


set(LODEPNG_SOURCE_FILES 

${CMAKE_CURRENT_LIST_DIR}/include/lodepng/lodepng.h
${CMAKE_CURRENT_LIST_DIR}/include/lodepng/lodepng.cpp

)


add_library(lodepng ${LODEPNG_SOURCE_FILES})
target_include_directories(lodepng PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/)