cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(drtd CXX)
set(OpenGL_GL_PREFERENCE GLVND)
set(CXXFLAGS
"-Wextra \
-Werror \
-Wall \
-Wshadow \
-Wnon-virtual-dtor \
-Wold-style-cast \
-Wcast-align \
-Wunused \
-Woverloaded-virtual \
-Wnull-dereference \
-Wduplicated-cond \
-Wduplicated-branches \
-Wlogical-op \
-Wuseless-cast \
-O2 -g -fno-exceptions -fno-rtti")
set(LDFLAGS "-g")

if ( PROFILE )
    message("Enabling profiling")
    set(CXXFLAGS "${CXXFLAGS} -pg -fno-omit-frame-pointer")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXXFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LDFLAGS}")
find_package(FLTK REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(src)
add_subdirectory(src)
set_target_properties(drtd PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}" )
