cmake_minimum_required(VERSION 3.7)
project(GOL)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_STANDARD 11)

# GMP
find_package(GMP REQUIRED)
message(STATUS "GMP library found.")
message(STATUS "GMP include dir is: ${GMP_INCLUDE_DIR}")
message(STATUS "GMP library is: ${GMP_LIBRARIES}")
include_directories(${GMP_INCLUDE_DIR})
set(MANDATORY_LIBRARIES ${GMP_LIBRARIES})

set(SOURCE_FILES main.cpp quad_tree.cpp quad_tree.h quad_tree_config.h quad_tree_node.cpp quad_tree_node.h quad_tree_tests.cpp quad_tree_tests.h)

add_executable(GOL ${SOURCE_FILES})
target_link_libraries(GOL gmp gmpxx)
