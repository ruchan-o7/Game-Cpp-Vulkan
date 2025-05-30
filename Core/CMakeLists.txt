project(Core
	LANGUAGES CXX
	VERSION 0.0.1)

message("----------")
message("Configuring ${PROJECT_NAME}")

find_package(Vulkan REQUIRED)

option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS  OFF)
option(ENTT_INCLUDE_NATVIS ON)

option(TINYGLTF_BUILD_LOADER_EXAMPLE ON)
option(JSON_Diagnostics ON)
add_subdirectory("./external/glfw")
add_subdirectory("./external/glm")
add_subdirectory("./external/entt")
add_subdirectory("./external/json")
add_subdirectory("./external/tinygltf")

set_property(TARGET "glfw" PROPERTY FOLDER "Dependencies")
set_property(TARGET "glm" PROPERTY FOLDER "Dependencies")
set_property(TARGET "EnTT" PROPERTY FOLDER "Dependencies")
set_property(TARGET "nlohmann_json" PROPERTY FOLDER "Dependencies")
set_property(TARGET "tinygltf" PROPERTY FOLDER "Dependencies")

file(GLOB_RECURSE CORE_SOURCES src/*.cpp src/*.h)

set(IMGUI_SRC_DIR "external/imgui")
set(SPDLOG_INCLUDE_DIR "external/spdlog")
set(GLFW_INCLUDE_DIR "external/glfw/include")
set(GLM_INCLUDE_DIR "external/glm/include")
set(STB_INCLUDE_DIR "external/stbi")
set(JSON_INCLUDE_DIR "external/json/include")
set(ENTT_INCLUDE_DIR "external/entt/src")
set(TINY_OBJ_LOADER_INCLUDE_DIR "external/tiny_obj_loader")
set(TINYGLTF_INCLUDE_DIR "external/tinygltf")

set(IMGUI_SRCS
	"${IMGUI_SRC_DIR}/imgui.cpp"
	"${IMGUI_SRC_DIR}/imgui_demo.cpp"
	"${IMGUI_SRC_DIR}/imgui_draw.cpp"
	"${IMGUI_SRC_DIR}/imgui_tables.cpp"
	"${IMGUI_SRC_DIR}/imgui_widgets.cpp"
	"${IMGUI_SRC_DIR}/imgui.h"
	"${IMGUI_SRC_DIR}/imgui_internal.h"
	"${IMGUI_SRC_DIR}/imstb_textedit.h"
	"${IMGUI_SRC_DIR}/imstb_truetype.h"
	"${IMGUI_SRC_DIR}/backends/imgui_impl_glfw.h"
	"${IMGUI_SRC_DIR}/backends/imgui_impl_glfw.cpp"
	"${IMGUI_SRC_DIR}/backends/imgui_impl_vulkan.h"
	"${IMGUI_SRC_DIR}/backends/imgui_impl_vulkan.cpp"
)

add_library("${PROJECT_NAME}" "${CORE_SOURCES};pch.cpp;${IMGUI_SRCS}")

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${GLFW_INCLUDE_DIR}"
	"${SPDLOG_INCLUDE_DIR}"
	"${GLM_INCLUDE_DIR}"
	"${STB_INCLUDE_DIR}"
	"${JSON_INCLUDE_DIR}"
	"${ENTT_INCLUDE_DIR}"
	"${IMGUI_SRC_DIR}"
	"${Vulkan_INCLUDE_DIRS}"
	"${TINYGLTF_INCLUDE_DIR}"
	"${TINY_OBJ_LOADER_INCLUDE_DIR}"
	"./"
)
add_compile_definitions(-DVK_NO_PROTOTYPES)
if(WIN32)
	add_compile_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif()

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)

target_link_libraries("${PROJECT_NAME}" PUBLIC
	glfw glm "${Vulkan_LIBRARIES}" EnTT::EnTT nlohmann_json
)

add_compile_definitions( $<$<CONFIG:Debug>:FOO_DEBUG>)

target_precompile_headers(Core PRIVATE "pch.h")

