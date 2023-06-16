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

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND) 
    message("ZLIB Found")
endif()

add_subdirectory("third_party_lib/cxxopts")

set(lg_path "/home/hkatehar/livegraph/LiveGraph/build")
link_directories(${lg_path})



add_executable(driver main.cpp livegraph_driver.cpp driver/sortledton_driver.cpp reader/graph_reader.cpp reader/log_reader.cpp stream/stream.cpp stream/update_stream.cpp experiments/algorithms_experiment.cpp experiments/updates_experiment.cpp experiments/mixed_experiment.cpp experiments/sequential_experiment.cpp experiments/sequentialv2_experiment.cpp experiments/mixedv2_experiment.cpp configuration.cpp)

target_include_directories(driver PUBLIC cxxopts)
target_include_directories(driver PUBLIC "/home/hkatehar/sortledton/sortledton/")


target_link_libraries(driver livegraph tbb cxxopts z "/home/hkatehar/sortledton/sortledton/build/libsortledton.a")