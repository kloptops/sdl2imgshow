cmake_minimum_required(VERSION 3.10)
project(sdl2imgshow)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Wextra -pedantic -Werror)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Find SDL2_image
find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})

# Find SDL2_ttf
find_package(SDL2_ttf REQUIRED)
include_directories(${SDL2_TTF_INCLUDE_DIRS})

add_executable(
    sdl2imgshow
    src/sdl2imgshow.c
    src/util.c
    )

# Link libraries
target_link_libraries(
    sdl2imgshow ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})
