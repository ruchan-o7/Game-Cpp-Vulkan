project(Editor
	LANGUAGES CXX
	VERSION 0.0.1)

message("----------")
message("Configuring ${PROJECT_NAME}")
set(LAYER_SOURCES 
	"src/Layer/EditorLayer.cpp"
	"src/Layer/Layer.cpp"
	"src/Layer/LayerStack.cpp"
)
set(EDITOR_SOURCES
	"src/Editor.h"
	"src/Editor.cpp"
	"src/EditorScene.h"
	"src/EditorScene.cpp"
	"src/EditorSceneDeserializer.h"
	"src/EditorSceneDeserializer.cpp"
	"src/main.cpp"
)
add_executable("${PROJECT_NAME}" "${EDITOR_SOURCES};${LAYER_SOURCES}")

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${ENTT_INCLUDE_DIR}"
	"${GLM_INCLUDE_DIR}"
	"${CORE_INCLUDE_DIR}"
	"${LOG_INCLUDE_DIR}"
	"${ENGINE_INCLUDE_DIR}"
	"${TINY_OBJ_LOADER_INCLUDE_DIR}"
	"src"
)

set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "Editor")

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)

target_link_libraries("${PROJECT_NAME}" Engine Core Log)
add_compile_definitions(
	$<$<CONFIG:Debug>:FOO_DEBUG>
)
