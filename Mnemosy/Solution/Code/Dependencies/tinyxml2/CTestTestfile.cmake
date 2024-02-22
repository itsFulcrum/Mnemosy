# CMake generated Testfile for 
# Source directory: C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Code/Dependencies/tinyxml2
# Build directory: C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/Code/Dependencies/tinyxml2
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(xmltest "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/bin/Debug/xmltest.exe")
  set_tests_properties(xmltest PROPERTIES  WORKING_DIRECTORY "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/bin/Debug" _BACKTRACE_TRIPLES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Code/Dependencies/tinyxml2/CMakeLists.txt;102;add_test;C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Code/Dependencies/tinyxml2/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(xmltest "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/bin/Release/xmltest.exe")
  set_tests_properties(xmltest PROPERTIES  WORKING_DIRECTORY "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/bin/Release" _BACKTRACE_TRIPLES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Code/Dependencies/tinyxml2/CMakeLists.txt;102;add_test;C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Code/Dependencies/tinyxml2/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(xmltest "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/bin/MinSizeRel/xmltest.exe")
  set_tests_properties(xmltest PROPERTIES  WORKING_DIRECTORY "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/bin/MinSizeRel" _BACKTRACE_TRIPLES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Code/Dependencies/tinyxml2/CMakeLists.txt;102;add_test;C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Code/Dependencies/tinyxml2/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(xmltest "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/bin/RelWithDebInfo/xmltest.exe")
  set_tests_properties(xmltest PROPERTIES  WORKING_DIRECTORY "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Solution/bin/RelWithDebInfo" _BACKTRACE_TRIPLES "C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Code/Dependencies/tinyxml2/CMakeLists.txt;102;add_test;C:/Users/fulcrum/Documents/cppProjects/Mnemosy/Mnemosy/Code/Dependencies/tinyxml2/CMakeLists.txt;0;")
else()
  add_test(xmltest NOT_AVAILABLE)
endif()
