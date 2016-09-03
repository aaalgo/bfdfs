cmake_minimum_required (VERSION 2.6)
project (BFDFS)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -msse2 -O3 -std=c++11")

find_package(Boost REQUIRED COMPONENTS filesystem system program_options)

add_executable(bfdfs-load bfdfs-load.cpp)
target_link_libraries(bfdfs-load ${Boost_LIBRARIES})
install (FILES bfdfs.h DESTINATION include) 
install (TARGETS bfdfs-load DESTINATION bin)
