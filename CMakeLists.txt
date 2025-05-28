cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(Game LANGUAGES CXX C)

add_subdirectory(Core)

add_subdirectory(Game)
add_subdirectory(Editor)

set_property(TARGET "Core" PROPERTY FOLDER "Core")
