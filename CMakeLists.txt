cmake_minimum_required(VERSION 3.18)

project(Driver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall")

# add_library( livegraph SHARED IMPORTED GLOBAL)
set(lg_path "/home/hkatehar/livegraph/LiveGraph/build")
link_directories(${lg_path})

add_executable(hello main.cpp livegraph_driver.cpp)

target_link_libraries(hello livegraph)