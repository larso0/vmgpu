cmake_minimum_required(VERSION 3.3)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${BP_ROOT_DIR}/CMakeModules")
project(vmgpu)

set(CMAKE_CXX_STANDARD 11)
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
elseif(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB_RECURSE SRC "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.c" "${SRC_DIR}/*.h" "${SRC_DIR}/*.hpp")

set(SHADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
file(GLOB_RECURSE SHADERS
     "${SHADERS_DIR}/*.vert"
     "${SHADERS_DIR}/*.tesc"
     "${SHADERS_DIR}/*.tese"
     "${SHADERS_DIR}/*.geom"
     "${SHADERS_DIR}/*.frag"
     "${SHADERS_DIR}/*.comp")

find_package(bp REQUIRED)
find_package(bpMulti REQUIRED)
find_package(bpQt REQUIRED)
find_package(bpScene REQUIRED)
find_package(Boost REQUIRED
	     COMPONENTS "program_options")

include_directories(${BP_INCLUDE_DIRS}
		    ${BP_MULTI_INCLUDE_DIRS}
		    ${BP_QT_INCLUDE_DIRS}
		    ${BP_SCENE_INCLUDE_DIRS}
		    ${Boost_INCLUDE_DIRS})

add_executable(vmgpu ${SRC})
target_link_libraries(vmgpu
		      ${BP_LIBRARIES}
		      ${BP_MULTI_LIBRARIES}
		      ${BP_QT_LIBRARIES}
		      ${BP_SCENE_LIBRARIES}
		      ${Boost_LIBRARIES})

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