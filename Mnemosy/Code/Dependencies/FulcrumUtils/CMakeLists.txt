cmake_minimum_required(VERSION 3.21)
project(Fulcrum_Utils)

Set(Fulcrum_Source_Files
include/FulcrumUtils/Flcrm_Log.hpp
src/Flcrm_Log.cpp
)


add_library(Fulcrum_Utils ${Fulcrum_Source_Files})
target_include_directories(Fulcrum_Utils PUBLIC include/)
