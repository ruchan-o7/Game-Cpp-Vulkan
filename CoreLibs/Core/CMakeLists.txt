project(Core
	LANGUAGES CXX
	VERSION 0.0.1)

message("----------")
message("Configuring ${PROJECT_NAME}")

find_package(Vulkan REQUIRED)
file(GLOB_RECURSE CORE_SOURCES
	"src/Scene/*.h"
	"src/Scene/*.cpp"
	"src/Scene/ObjectConverters/*.h"
	"src/Scene/ObjectConverters/*.cpp"

	"src/ImGui/*.cpp"

	"src/Core/*.h"
	"src/Core/*.cpp"
	
	"src/Input/*.h"
	"src/Input/*.cpp"

	"src/Resouces/*.cpp"
)
file(GLOB_RECURSE SCRIPTS
	"src/Scripts/*.cpp"
	"src/Scripts/*.h"
	)
file(GLOB_RECURSE ENGINE_SOURCES
	"src/Engine/Camera/*.h"
	"src/Engine/Camera/*.cpp"

	"src/Engine/Core/*.h"
	"src/Engine/Core/*.cpp"
	
	"src/Engine/Engine/*.h"
	"src/Engine/Engine/*.cpp"
	
	"src/Engine/Geometry/*.h"
	"src/Engine/Events/*.cpp"
	
	"src/Engine/Geometry/*.cpp"
	"src/Engine/Geometry/*.cpp"
	
	"src/Engine/Implementations/*.cpp"

	"src/Engine/Input/*.cpp"
	"src/Engine/Input/*.cpp"

	"src/Engine/Types/*.cpp"
	"src/Engine/Types/*.cpp"

	"src/Engine/Window/*.cpp"
	"src/Engine/Window/*.cpp"

)
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
add_library("${PROJECT_NAME}" "${CORE_SOURCES};${ENGINE_SOURCES};pch.cpp;${IMGUI_SRCS};${SCRIPTS}")

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${GLFW_INCLUDE_DIR}"
	"${GLM_INCLUDE_DIR}"
	"${STB_INCLUDE_DIR}"
	"${JSON_INCLUDE_DIR}"
	"${ENTT_INCLUDE_DIR}"
	"${LOG_INCLUDE_DIR}"
	"${IMGUI_SRC_DIR}"
	"${Vulkan_INCLUDE_DIRS}"
	"${TINYGLTF_INCLUDE_DIR}"
	"${TINY_OBJ_LOADER_INCLUDE_DIR}"
	"./"
)


set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)
target_link_libraries("${PROJECT_NAME}" PUBLIC
	glfw glm Log
	"${Vulkan_LIBRARIES}"
	 EnTT::EnTT nlohmann_json
)

add_compile_definitions(
	$<$<CONFIG:Debug>:FOO_DEBUG>
)
target_precompile_headers(Core PRIVATE "pch.h")
