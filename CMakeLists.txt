cmake_minimum_required(VERSION 4.0)
project(mgmake)

set(CMAKE_CXX_STANDARD 26)

add_library(mgmake INTERFACE)
target_include_directories(mgmake INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

add_executable(builder)
target_sources(builder PRIVATE build.cxx)
target_link_libraries(builder PRIVATE mgmake)
