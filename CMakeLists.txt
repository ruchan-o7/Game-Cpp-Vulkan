cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Game
	LANGUAGES CXX C)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)


option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS  OFF)

option(VK_BOOTSTRAP_WERR  ON)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)

set_property(TARGET "glfw" PROPERTY FOLDER "Dependencies")
set_property(TARGET "glm" PROPERTY FOLDER "Dependencies")

set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")

set(GLFW_INCLUDE_DIR "${EXTERNAL_DIR}/glfw/include")
set(GLM_INCLUDE_DIR "${EXTERNAL_DIR}/glm/include")
set(STB_INCLUDE_DIR "${EXTERNAL_DIR}/stbi")
set(IMGUI_SRC_DIR "${EXTERNAL_DIR}/imgui")
add_subdirectory(Core)

add_subdirectory(Game)
