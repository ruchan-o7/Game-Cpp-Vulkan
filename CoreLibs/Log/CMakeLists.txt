project(Log LANGUAGES CXX)

message("----------")
message("Configuring ${PROJECT_NAME}")

file(GLOB_RECURSE LOG_SOURCE "src/*.cpp" "src/*.h")

add_library("${PROJECT_NAME}" STATIC "${LOG_SOURCE}")

add_compile_definitions(
    $<$<CONFIG:Debug>:FOO_DEBUG>
)
set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 17)

target_include_directories("${PROJECT_NAME}" PUBLIC
	"${SPDLOG_INCLUDE_DIR}"
)
set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "CoreLibs")
