cmake_minimum_required(VERSION 3.15)
project(fixed_xml_parser CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(tinyxml2 REQUIRED)
find_package(expat REQUIRED)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE tinyxml2::tinyxml2 expat::expat)
