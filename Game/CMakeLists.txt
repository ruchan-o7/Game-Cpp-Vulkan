project(Game
	LANGUAGES CXX)

message("----------")
message("Configuring ${PROJECT_NAME}")

set(GAME_SOURCES
	"src/main.cpp"
	"src/Game.cpp"
	)

add_executable("${PROJECT_NAME}" "${GAME_SOURCES}")

set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "Game")
set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)

target_link_libraries("${PROJECT_NAME}" PRIVATE Core )

target_include_directories("${PROJECT_NAME}" PRIVATE
	"${CORE_INCLUDE_DIR}"
	"${Vulkan_INCLUDE_DIR}"
	"${IMGUI_SRC_DIR}"
)
