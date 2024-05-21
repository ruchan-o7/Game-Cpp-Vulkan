cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Game
	LANGUAGES CXX C)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(FOO_DEBUG)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS  OFF)
option(ENTT_INCLUDE_NATVIS ON)

option(JSON_Diagnostics ON)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/entt)
add_subdirectory(external/json)

set_property(TARGET "glfw" PROPERTY FOLDER "Dependencies")
set_property(TARGET "glm" PROPERTY FOLDER "Dependencies")
set_property(TARGET "EnTT" PROPERTY FOLDER "Dependencies")
set_property(TARGET "nlohmann_json" PROPERTY FOLDER "Dependencies")

set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")

set(SPDLOG_INCLUDE_DIR "${EXTERNAL_DIR}/glfw/spdlog")
set(GLFW_INCLUDE_DIR "${EXTERNAL_DIR}/glfw/include")
set(GLM_INCLUDE_DIR "${EXTERNAL_DIR}/glm/include")
set(STB_INCLUDE_DIR "${EXTERNAL_DIR}/stbi")
set(JSON_INCLUDE_DIR "${EXTERNAL_DIR}/json/include")
set(ENTT_INCLUDE_DIR "${EXTERNAL_DIR}/entt/src")
set(IMGUI_SRC_DIR "${EXTERNAL_DIR}/imgui")
set(TINY_OBJ_LOADER_INCLUDE_DIR "${EXTERNAL_DIR}/tiny_obj_loader")
set(ENGINE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/Engine")
set(CORE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/Core/src")

add_subdirectory(Engine)
add_subdirectory(Editor)

add_subdirectory(Core)
add_subdirectory(Game)
