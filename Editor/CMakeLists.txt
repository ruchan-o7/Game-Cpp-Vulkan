project(Editor
	LANGUAGES CXX
	VERSION 0.0.1)

message("----------")
message("Configuring ${PROJECT_NAME}")

set(EDITOR_SOURCES
	"src/Core/LayerStack.cpp"
	"src/Core/LayerStack.h"
	"src/Core/EditorLayer.h"
	"src/Core/EditorLayer.cpp"
	"src/Core/Layer.h"
	"src/Core/Layer.cpp"
	"src/Editor.h"
	"src/Editor.cpp"
	"src/EditorScene.h"
	"src/EditorScene.cpp"
	"src/main.cpp"
)
add_executable("${PROJECT_NAME}" "${EDITOR_SOURCES}")

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${Vulkan_INCLUDE_DIRS}"
	"${GLFW_INCLUDE_DIR}"
	"${ENTT_INCLUDE_DIR}"
	"${GLM_INCLUDE_DIR}"
	"${CORE_INCLUDE_DIR}"
	"${LOG_INCLUDE_DIR}"
	"${STB_INCLUDE_DIR}"
	"${IMGUI_SRC_DIR}"
	"${ENGINE_INCLUDE_DIR}"
	"${TINY_OBJ_LOADER_INCLUDE_DIR}"
	"src"
)

set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "CoreLibs")

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)

target_link_libraries("${PROJECT_NAME}" Engine Core Log)
add_compile_definitions(
	$<$<CONFIG:Debug>:FOO_DEBUG>
)
