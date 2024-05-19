project(Editor
	LANGUAGES CXX
	VERSION 0.0.1)

message("----------")
message("Configuring ${PROJECT_NAME}")

set(EDITOR_SOURCES
	"src/Core/Layer.h"
	"src/Core/Layer.cpp"
	"src/Editor.h"
	"src/Editor.cpp"
	"src/main.cpp"
)
add_executable("${PROJECT_NAME}" "${EDITOR_SOURCES}")

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${Vulkan_INCLUDE_DIRS}"
	"${GLFW_INCLUDE_DIR}"
	"${ENTT_INCLUDE_DIR}"
	"${GLM_INCLUDE_DIR}"
	"${STB_INCLUDE_DIR}"
	"${IMGUI_SRC_DIR}"
	"${ENGINE_INCLUDE_DIR}"
	"${TINY_OBJ_LOADER_INCLUDE_DIR}"
	"src"
)

set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "Editor")

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)

target_link_libraries("${PROJECT_NAME}" Engine Core)
