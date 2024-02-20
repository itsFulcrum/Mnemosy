# Install script for directory: C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Code/Dependencies/tinyxml2

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/MnemosyEngine")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "tinyxml2_libraries" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/Code/Dependencies/tinyxml2/Debug/tinyxml2d.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/Code/Dependencies/tinyxml2/Release/tinyxml2.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/Code/Dependencies/tinyxml2/MinSizeRel/tinyxml2.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/Code/Dependencies/tinyxml2/RelWithDebInfo/tinyxml2.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "tinyxml2_headers" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Code/Dependencies/tinyxml2/tinyxml2.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "tinyxml2_config" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/Code/Dependencies/tinyxml2/tinyxml2.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "tinyxml2_config" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/MnemosyApplication" TYPE FILE FILES
    "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/MnemosyApplicationConfig.cmake"
    "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/MnemosyApplicationConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "tinyxml2_config" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/MnemosyApplication/MnemosyApplicationTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/MnemosyApplication/MnemosyApplicationTargets.cmake"
         "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/Code/Dependencies/tinyxml2/CMakeFiles/Export/90105bb8a0cfe6d87d82872d021808fc/MnemosyApplicationTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/MnemosyApplication/MnemosyApplicationTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/MnemosyApplication/MnemosyApplicationTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/MnemosyApplication" TYPE FILE FILES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/Code/Dependencies/tinyxml2/CMakeFiles/Export/90105bb8a0cfe6d87d82872d021808fc/MnemosyApplicationTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/MnemosyApplication" TYPE FILE FILES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/Code/Dependencies/tinyxml2/CMakeFiles/Export/90105bb8a0cfe6d87d82872d021808fc/MnemosyApplicationTargets-debug.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/MnemosyApplication" TYPE FILE FILES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/Code/Dependencies/tinyxml2/CMakeFiles/Export/90105bb8a0cfe6d87d82872d021808fc/MnemosyApplicationTargets-minsizerel.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/MnemosyApplication" TYPE FILE FILES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/Code/Dependencies/tinyxml2/CMakeFiles/Export/90105bb8a0cfe6d87d82872d021808fc/MnemosyApplicationTargets-relwithdebinfo.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/MnemosyApplication" TYPE FILE FILES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/Code/Dependencies/tinyxml2/CMakeFiles/Export/90105bb8a0cfe6d87d82872d021808fc/MnemosyApplicationTargets-release.cmake")
  endif()
endif()

