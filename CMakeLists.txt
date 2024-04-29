cmake_minimum_required(VERSION 3.25)

set_property(GLOBAL
	PROPERTY CMAKE_EXPORT_COMPILE_COMMANDS
	VALUE ON)

project(Game
	LANGUAGES CXX C)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(Core)

add_subdirectory(Game)
