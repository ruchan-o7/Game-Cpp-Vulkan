project(Engine LANGUAGES CXX)

message("----------")
message("Configuring ${PROJECT_NAME}")


file(GLOB ENGINE_SOURCE "src/Engine/*.cpp" "src/Engine/*.h")
file(GLOB CAMERA_SOURCE "src/Camera/*.cpp" "src/Camera/*.h")
set(WINDOW_SOURCE "src/Window/Window.h;src/Window/Window.cpp")
set(GEOMETRY_SOURCE
	"src/Geometry/Model.h;src/Geometry/Vertex.h;src/Geometry/Vertex.cpp;src/Geometry/Model.cpp")

set(ENGINE_SOURCES
	"${ENGINE_SOURCE}"
	"${CAMERA_SOURCE}"
	"${WINDOW_SOURCE}"
	"${GEOMETRY_SOURCE}"
)

message("Engine Founded resources: ${ENGINE_SOURCES}")

add_library("${PROJECT_NAME}" STATIC "${ENGINE_SOURCES}")

find_package(Vulkan REQUIRED)

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)

target_link_libraries("${PROJECT_NAME}" PUBLIC glfw glm "${Vulkan_LIBRARIES}")

target_include_directories("${PROJECT_NAME}" PRIVATE
	"${Vulkan_INCLUDE_DIRS}"
	"${STB_INCLUDE_DIR}"
	"${IMGUI_SRC_DIR}"
	"${GLFW_INCLUDE_DIR}"
	"${GLM_INCLUDE_DIR}"
)

set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "Engine")
