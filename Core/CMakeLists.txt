project(Core
	LANGUAGES CXX
	VERSION 0.0.1)

message("----------")
message("Configuring ${PROJECT_NAME}")

find_package(Vulkan REQUIRED)

set(CORE_SOURCES
	"src/Core/demo.cpp"
	"src/Core/Window.cpp"
	)

add_library("${PROJECT_NAME}" "${CORE_SOURCES}")


target_include_directories("${PROJECT_NAME}" PRIVATE
	"${Vulkan_INCLUDE_DIRS}"
	"${GLFW_INCLUDE_DIR}"
	"${GLM_INCLUDE_DIR}"
	)

set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "Core")

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)
target_link_libraries("${PROJECT_NAME}" PUBLIC glfw glm "${Vulkan_LIBRARIES}")