project(Core
	LANGUAGES CXX
	VERSION 0.0.1)

message("----------")
message("Configuring ${PROJECT_NAME}")

find_package(Vulkan REQUIRED)

set(CORE_SOURCES
	"src/pch.cpp"
	"src/Core/Core/Window.cpp"
	"src/Core/Core/Engine.h"
	"src/Core/Core/Engine.cpp"

	"src/Core/File/FileHelper.cpp"
	"src/Core/Input/Input.cpp"
	"src/Core/Backend/VulkanCheckResult.cpp"

	"src/Core/Graphics/Api.h"
	"src/Core/Graphics/Buffer.h"
	"src/Core/Graphics/Command.h"
	"src/Core/Graphics/Device.h"
	"src/Core/Graphics/Image.h"
	"src/Core/Graphics/Semaphore.h"
	"src/Core/Graphics/Shader.h"
	"src/Core/Graphics/Swapchain.h"

	"src/Core/Graphics/Api.cpp"
	"src/Core/Graphics/Buffer.cpp"
	"src/Core/Graphics/Command.cpp"
	"src/Core/Graphics/Device.cpp"
	"src/Core/Graphics/Image.cpp"
	"src/Core/Graphics/Semaphore.cpp"
	"src/Core/Graphics/Shader.cpp"
	"src/Core/Graphics/Swapchain.cpp"
	"src/Core/Graphics/stb_image.cpp"
)

add_library("${PROJECT_NAME}" "${CORE_SOURCES}")

target_include_directories("${PROJECT_NAME}" PRIVATE
	"${Vulkan_INCLUDE_DIRS}"
	"${GLFW_INCLUDE_DIR}"
	"${GLM_INCLUDE_DIR}"
	"${VK_BOOTSTRAP_INCLUDE_DIR}"
	"${STB_INCLUDE_DIR}"
	"src"
)

set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "Core")

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)
target_link_libraries("${PROJECT_NAME}" PUBLIC glfw glm "${Vulkan_LIBRARIES}")

target_precompile_headers(Core PRIVATE "src/pch.h")
