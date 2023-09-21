cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

project("wui_example" CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})

#source files
file(GLOB_RECURSE sources src/*.cpp)
add_executable(${PROJECT_NAME} ${sources})

#lib files
add_subdirectory(libs)

# copy ui files
cef_copy_ui_files("html")
cef_copy_dependencies(${PROJECT_NAME} "libs/wui")

#header files 
target_include_directories(${PROJECT_NAME} PRIVATE include)


# Libraries
find_package(spdlog REQUIRED)
find_package(Threads REQUIRED)


target_link_libraries(${PROJECT_NAME} PRIVATE 
  allegro
  allegro_image
  allegro_primitives
  allegro_ttf
  allegro_font
  allegro_dialog
  spdlog::spdlog
  Threads::Threads)


