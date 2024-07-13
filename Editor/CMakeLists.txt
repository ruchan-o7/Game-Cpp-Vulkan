project(Editor
	LANGUAGES CXX
	VERSION 0.0.1)

message("----------")
message("Configuring ${PROJECT_NAME}")
file(GLOB_RECURSE EDITOR_SOURCES
	"src/*.h"
	"src/*.cpp"
	"src/Layer/*.cpp"
	"src/Panels/*.cpp"
)
add_executable("${PROJECT_NAME}" "${EDITOR_SOURCES};")

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${ENTT_INCLUDE_DIR}"
	"${GLM_INCLUDE_DIR}"
	"${CORE_INCLUDE_DIR}"
	"${LOG_INCLUDE_DIR}"
	"${TINY_OBJ_LOADER_INCLUDE_DIR}"
)

set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "Editor")

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)

target_link_libraries("${PROJECT_NAME}" Core Log)

add_compile_definitions(
	$<$<CONFIG:Debug>:FOO_DEBUG>
)
