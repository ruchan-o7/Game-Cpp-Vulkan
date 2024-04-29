project(Core
	LANGUAGES CXX
	VERSION 0.0.1)

message("----------")
message("Configuring ${PROJECT_NAME}")

set(CORE_SOURCES
	"src/Core/demo.cpp")

add_library("${PROJECT_NAME}" "${CORE_SOURCES}")

set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "Core")

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)
