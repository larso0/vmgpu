set(BP_MODULES "Multi;Qt;Scene")
add_subdirectory(bp)

cmake_minimum_required(VERSION 3.3)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "bp/CMakeModules")
project(vmgpu)

set(CMAKE_CXX_STANDARD 11)
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
elseif(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

file(GLOB_RECURSE SRC "src/*.cpp" "src/*.c" "src/*.h" "src/*.hpp")

file(GLOB_RECURSE SHADERS
     "shaders/*.vert"
     "shaders/*.tesc"
     "shaders/*.tese"
     "shaders/*.geom"
     "shaders/*.frag"
     "shaders/*.comp")

find_package(Boost REQUIRED COMPONENTS "program_options" "filesystem")

add_executable(vmgpu ${SRC})
target_include_directories(vmgpu PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(vmgpu bp bpMulti bpQt bpScene ${Boost_LIBRARIES})

find_program(GLSLANG_VALIDATOR glslangValidator)
foreach(SHADER ${SHADERS})
	get_filename_component(FILE_NAME ${SHADER} NAME)
	set(SPIRV "${PROJECT_BINARY_DIR}/spv/${FILE_NAME}.spv")
	add_custom_command(
		OUTPUT ${SPIRV}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/spv/"
		COMMAND ${GLSLANG_VALIDATOR} -V ${SHADER} -o ${SPIRV}
		DEPENDS ${SHADER})
	list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(SHADER)

add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(vmgpu shaders)