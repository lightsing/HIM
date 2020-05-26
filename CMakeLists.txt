cmake_minimum_required(VERSION 3.16)
project(HIM VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
add_subdirectory(vendor)

include_directories(utils vendor/stb)

file(GLOB SOURCE src/*.h src/*.cpp utils/learnopengl/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCE} src/maze.cpp src/maze.h)
target_include_directories(${PROJECT_NAME} PUBLIC external ${OPENGL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_gl_LIBRARY} glfw glad glm assimp)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")