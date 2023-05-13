#
# Some Notes
#
# debug build:      cmake -DCMAKE_BUILD_TYPE=Debug .
# verbose make:     make VERBOSE=1
#

cmake_minimum_required(VERSION 3.7)

project(gbemu)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(SDL2 REQUIRED)
find_package(SDL2_gfx REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

find_package (Threads)

FILE(GLOB MySources src/*.cc src/*/*.cc)
add_executable(gbemu ${MySources})
target_link_libraries(gbemu ${SDL2_LIBRARIES} ${SDL2_GFX_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

