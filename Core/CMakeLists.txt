project(Core
	LANGUAGES CXX
	VERSION 0.0.1)

message("----------")
message("Configuring ${PROJECT_NAME}")

find_package(Vulkan REQUIRED)

set(CORE_SOURCES
	"src/pch.cpp"
	"src/Core/Core/Window.cpp"
	"src/Core/Core/OrthographicCamera.cpp"
	"src/Core/Core/OrthographicCamera.h"
	"src/Core/Core/Engine.h"
	"src/Core/Core/Engine.cpp"
	"src/Core/Core/PerspectiveCamera.cpp"

	"src/Core/File/FileHelper.cpp"
	"src/Core/Input/Input.cpp"
	"src/Core/Backend/VulkanCheckResult.cpp"

	"src/Core/Graphics/Api.h"
	"src/Core/Graphics/Types/QuadVertex.h"
	"src/Core/Graphics/Buffer.h"
	"src/Core/Graphics/Command.h"
	"src/Core/Graphics/Device.h"
	"src/Core/Graphics/Texture2D.h"
	"src/Core/Graphics/Semaphore.h"
	"src/Core/Graphics/Shader.h"
	"src/Core/Graphics/Swapchain.h"
	"src/Core/Graphics/Model.h"
	"src/Core/Graphics/Pipeline.h"
	"src/Core/Graphics/Renderer2D.h"
	"src/Core/Graphics/Renderer3D.h"
	"src/Core/Graphics/Descriptor/DescriptorAllocator.h"

	"src/Core/Scene/Component.h"
	"src/Core/Scene/GameObject.h"
	"src/Core/Scene/Scene.h"

	"src/Core/Graphics/Api.cpp"
	"src/Core/Graphics/Buffer.cpp"
	"src/Core/Graphics/Command.cpp"
	"src/Core/Graphics/Device.cpp"
	"src/Core/Graphics/Texture2D.cpp"
	"src/Core/Graphics/Semaphore.cpp"
	"src/Core/Graphics/Shader.cpp"
	"src/Core/Graphics/Swapchain.cpp"
	"src/Core/Graphics/stb_image.cpp"
	"src/Core/Backend/TinyObjLoader.cpp"
	"src/Core/Graphics/Model.cpp"
	"src/Core/Graphics/Pipeline.cpp"
	"src/Core/Graphics/Renderer2D.cpp"
	"src/Core/Graphics/Renderer3D.cpp"
	"src/Core/Graphics/Descriptor/DescriptorAllocator.cpp"

	"src/Core/Scene/SampleScene.cpp"
	"src/Core/Scene/GameObject.cpp"
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
list(APPEND "${CORE_SOURCES}" "${IMGUI_SRCS}")

add_library("${PROJECT_NAME}" "${CORE_SOURCES}""${IMGUI_SRCS}")

target_include_directories("${PROJECT_NAME}" PRIVATE
	"${Vulkan_INCLUDE_DIRS}"
	"${GLFW_INCLUDE_DIR}"
	"${GLM_INCLUDE_DIR}"
	"${VK_BOOTSTRAP_INCLUDE_DIR}"
	"${STB_INCLUDE_DIR}"
	"${IMGUI_SRC_DIR}"
	"${TINY_OBJ_LOADER_INCLUDE_DIR}"
	"src"
)

set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "Core")

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)
target_link_libraries("${PROJECT_NAME}" PUBLIC glfw glm "${Vulkan_LIBRARIES}")

target_precompile_headers(Core PRIVATE "src/pch.h")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(FOO_DEBUG)
endif()
add_compile_definitions($<$<CONFIG:Debug>:FOO_DEBUG>)
