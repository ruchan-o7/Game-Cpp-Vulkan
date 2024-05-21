project(Core
	LANGUAGES CXX
	VERSION 0.0.1)

message("----------")
message("Configuring ${PROJECT_NAME}")

find_package(Vulkan REQUIRED)

set(CORE_SOURCES
	"src/pch.cpp"

	"src/Core/File/FileHelper.cpp"

	"src/Core/Backend/TinyObjLoader.cpp"

	"src/Core/Core/UUID.cpp"
	"src/Core/Core/Log.cpp"

	"src/Core/Scene/Component.h"
	"src/Core/Scene/ScriptableEntity.h"
	"src/Core/Scene/Scene.h"
	"src/Core/Scene/Scene.cpp"
	"src/Core/Scene/Entity.h"
	"src/Core/Scene/Entity.cpp"

	"src/Core/Resources/LoadModel.h"
	"src/Core/Resources/LoadModel.cpp"
)
add_library("${PROJECT_NAME}" "${CORE_SOURCES}")

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${SPDLOG_INCLUDE_DIR}"
	"${Vulkan_INCLUDE_DIRS}"
	"${GLFW_INCLUDE_DIR}"
	"${ENTT_INCLUDE_DIR}"
	"${GLM_INCLUDE_DIR}"
	"${STB_INCLUDE_DIR}"
	"${JSON_INCLUDE_DIR}"
	"${IMGUI_SRC_DIR}"
	"${ENGINE_INCLUDE_DIR}"
	"${TINY_OBJ_LOADER_INCLUDE_DIR}"
	"src"
)

set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "Core")

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)
target_link_libraries("${PROJECT_NAME}"
	PUBLIC glfw glm
	"${Vulkan_LIBRARIES}"
	Engine EnTT::EnTT nlohmann_json
)

add_compile_definitions(
	$<$<CONFIG:Debug>:FOO_DEBUG>
)
target_precompile_headers(Core PRIVATE "src/pch.h")
