cmake_minimum_required(VERSION 3.18)

project(Driver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall")

find_package(OpenMP)
if (OPENMP_FOUND)
    message("OpenMP Found")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

find_package(TBB REQUIRED)
if(TBB_FOUND) 
    message("TBB Found")
endif()
include_directories(${TBB_INCLUDE_DIRS})
link_directories(${TBB_LIBRARY_DIRS})
link_libraries(${TBB_LIBRARIES})

set(lg_path "/home/hkatehar/livegraph/LiveGraph/build")
link_directories(${lg_path})
link_directories("reader")

add_executable(hello main.cpp livegraph_driver.cpp reader/reader.cpp stream/stream.cpp configuration.cpp)

target_link_libraries(hello livegraph tbb)