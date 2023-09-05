cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

project("wui_example" CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)




#source files
add_executable(${PROJECT_NAME} src/main.cpp)

#lib files
add_subdirectory(libs)

#header files 
target_include_directories(${PROJECT_NAME} PRIVATE include/)


# Libraries
find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE 
  allegro
  allegro_image
  allegro_primitives
  allegro_ttf
  allegro_font
  allegro_dialog)


