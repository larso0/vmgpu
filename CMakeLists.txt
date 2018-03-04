cmake_minimum_required(VERSION 3.9)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${BP_ROOT_DIR}/CMakeModules")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
project(vmgpu)

set(CMAKE_CXX_STANDARD 11)

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
find_package(bpScene REQUIRED)
find_package(bpView REQUIRED)
find_package(Boost REQUIRED
	     COMPONENTS "program_options")

if(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

if (NOT BP_FOUND)
	message(SEND_ERROR "bp not found")
endif()
if (NOT BP_SCENE_FOUND)
	message(SEND_ERROR "bpScene not found")
endif()
if (NOT BP_VIEW_FOUND)
	message(SEND_ERROR "bpView not found")
endif()
if (NOT Boost_FOUND)
	message(SEND_ERROR "Required Boost libraries not found.")
endif()

include_directories(${BP_INCLUDE_DIRS}
		    ${BP_SCENE_INCLUDE_DIRS}
		    ${BP_VIEW_INCLUDE_DIRS}
		    ${Boost_INCLUDE_DIRS})

add_executable(vmgpu ${SRC} src/subpasses/SFBenchSubpass.cpp src/subpasses/SFBenchSubpass.h)
target_link_libraries(vmgpu
		      ${BP_LIBRARIES}
		      ${BP_SCENE_LIBRARIES}
		      ${BP_VIEW_LIBRARIES}
		      ${Boost_LIBRARIES})

if (NOT GLSLANG_VALIDATOR)
	set(GLSLANG_VALIDATOR glslangValidator)
endif()
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
