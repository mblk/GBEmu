cmake_minimum_required(VERSION 3.7)

project(gbemu)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package (Threads)

FILE(GLOB MySources src/*.cc src/*/*.cc)
add_executable(gbemu ${MySources})
target_link_libraries(gbemu ${SDL2_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

