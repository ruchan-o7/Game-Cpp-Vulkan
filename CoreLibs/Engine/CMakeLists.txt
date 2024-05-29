project(Engine LANGUAGES CXX)

message("----------")
message("Configuring ${PROJECT_NAME}")


file(GLOB_RECURSE ENGINE_SOURCE "src/Engine/*.cpp" "src/Engine/*.h")

file(GLOB_RECURSE INPUT_SOURCE "src/Input/*.cpp" "src/Input/*.h")

file(GLOB CAMERA_SOURCE "src/Camera/*.cpp" "src/Camera/*.h")

set(WINDOW_SOURCE "src/Window/Window.h;src/Window/Window.cpp")
set(GEOMETRY_SOURCE
	"src/Geometry/Model.h"
	"src/Geometry/Vertex.h"
	"src/Geometry/Vertex.cpp"
	"src/Geometry/Model.cpp"
	"src/Geometry/Mesh.cpp"
	"src/Geometry/AssetLoader.cpp"
	)
set(LIBS_IMPLS 
 	"src/Implementations/tiny_gltf.cpp"
	"src/Implementations/tiny_obj_loader.cpp"
	"src/Implementations/stb_image.cpp"
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
set(ENGINE_SOURCES
	"${ENGINE_SOURCE}"
	"${IMGUI_SRCS}"
	"${INPUT_SOURCE}"
	"${CAMERA_SOURCE}"
	"${WINDOW_SOURCE}"
	"${GEOMETRY_SOURCE}"
	"${ILIBS_IMPLSMGUI_SRCS}"
	"${LIBS_IMPLS}"
	"src/Engine/Descriptor/DescriptorAllocator.cpp"
)

add_library("${PROJECT_NAME}" STATIC "${ENGINE_SOURCES}")

find_package(Vulkan REQUIRED)

add_compile_definitions(
$<$<CONFIG:Debug>:FOO_DEBUG>
)
set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)

target_link_libraries("${PROJECT_NAME}" PUBLIC
	glfw glm Log "${Vulkan_LIBRARIES}" 
	)

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${Vulkan_INCLUDE_DIRS}"
	"${TINYGLTF_INCLUDE_DIR}"
	"${TINY_OBJ_LOADER_INCLUDE_DIR}"
	"${LOG_INCLUDE_DIR}"
	"${STB_INCLUDE_DIR}"
	"${IMGUI_SRC_DIR}"
	"${GLFW_INCLUDE_DIR}"
	"${GLM_INCLUDE_DIR}"
)
