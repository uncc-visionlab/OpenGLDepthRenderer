cmake_minimum_required (VERSION 3.0)
cmake_policy(VERSION 3.0)

project (OpenGLDepthRenderer)

SET(CMAKE_VERBOSE_MAKEFILE ON)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

if(WIN32)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
endif(WIN32)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/lib)

list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -no-pie -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no-pie -fPIC")

# find the required packages
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")
# find_package(SOIL REQUIRED)
# message(STATUS "Found SOIL in ${SOIL_INCLUDE_DIR}")
# find_package(GLEW REQUIRED)
# message(STATUS "Found GLEW in ${GLEW_INCLUDE_DIR}")

if(WIN32)
  set(LIBS glfw3 opengl32 assimp freetype irrKlang)
elseif(UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
  find_package(OpenGL REQUIRED)
  add_definitions(${OPENGL_DEFINITIONS})
  find_package(X11 REQUIRED)
  # note that the order is important for setting the libs
  # use pkg-config --libs $(pkg-config --print-requires --print-requires-private glfw3) in a terminal to confirm
  set(LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread freetype ${ASSIMP_LIBRARY})
  set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
elseif(APPLE)
  INCLUDE_DIRECTORIES(/System/Library/Frameworks)
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
  FIND_LIBRARY(IOKit_LIBRARY IOKit)
  FIND_LIBRARY(CoreVideo_LIBRARY CoreVideo)
  MARK_AS_ADVANCED(COCOA_LIBRARY OpenGL_LIBRARY)
  SET(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
  SET(APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY})
  set(LIBS ${LIBS} ${APPLE_LIBS})
else()
  set(LIBS )
endif(WIN32)


configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_SOURCE_DIR}/configuration)

# first create relevant static libraries requried for other projects
add_library(STB_IMAGE "src/stb_image.cpp")
set(LIBS ${LIBS} STB_IMAGE)

add_library(GLAD "src/glad.c")
set(LIBS ${LIBS} GLAD)

add_library(YAMLCONFIG "src/YAML_Config.cpp")
target_link_libraries(YAMLCONFIG yaml-cpp)
set(LIBS ${LIBS} YAMLCONFIG)

add_library(MISC src/model_export.cpp src/screenshots.cpp)
set(LIBS ${LIBS} MISC)

#########################################################
# Build third party support library
# yaml-cpp -> https://github.com/jbeder/yaml-cpp yaml-cpp is a YAML parser and emitter in C++ matching the YAML 1.2 spec.
add_subdirectory(third_party)

macro(makeLink src dest target)
  add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dest}  DEPENDS  ${dest} COMMENT "mklink ${src} -> ${dest}")
endmacro()

# then create a project file per tutorial
file(GLOB SOURCE
    "src/*.h"
    "src/*.vs"
    "src/*.fs"
    "src/*.gs"
)
set(NAMES OpenGLDepthRenderer OpenGLModelRenderer ogl_depthrenderer ogl_ML_data_augmenter)
foreach(NAME ${NAMES})
    add_executable(${NAME} src/${NAME}.cpp ${SOURCE})
    target_link_libraries(${NAME} ${LIBS} png)
    if(WIN32)
        set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/")
        set_target_properties(${NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/Debug")
    elseif(UNIX AND NOT APPLE)
        set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/")
    elseif(APPLE)
        set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/")
        set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/bin/")
        set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/bin/")
    endif(WIN32)
endforeach(NAME)

# copy shader files to build directory
file(GLOB SHADERS
         "src/*.vs"
         # "src/${CHAPTER}/${DEMO}/*.frag"
         "src/*.fs"
         "src/*.gs"
)

foreach(SHADER ${SHADERS})
    if(WIN32)
        # configure_file(${SHADER} "test")
        add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    elseif(UNIX AND NOT APPLE)
        #file(COPY ${SHADER} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin/)
        #add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        #    #COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}
        #    COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} ${CMAKE_CURRENT_BINARY_DIR}/bin/
        #)
        #configure_file(${SHADER} ${CMAKE_CURRENT_BINARY_DIR}/bin/${SHADER} COPYONLY)
        get_filename_component(SHADER_FILENAME "${SHADER}" NAME)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_FILENAME}
            COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} ${CMAKE_CURRENT_BINARY_DIR}/bin/${SHADER_FILENAME}
            MAIN_DEPENDENCY ${SHADER})
    elseif(APPLE)
        # create symbolic link for *.vs *.fs *.gs
        get_filename_component(SHADERNAME ${SHADER} NAME)
        makeLink(${SHADER} ${CMAKE_CURRENT_BINARY_DIR}/bin/${SHADERNAME} ${PROJECT_NAME})
    endif(WIN32)
endforeach(SHADER)

# if compiling for visual studio, also use configure file for each project (specifically to set up working directory)
if(MSVC)
    configure_file(${CMAKE_SOURCE_DIR}/configuration/visualstudio.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.vcxproj.user @ONLY)
endif(MSVC)


include_directories(${CMAKE_SOURCE_DIR}/include)
