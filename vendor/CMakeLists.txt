cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 11)

set(GLFW_LIB_NAME glfw)
set(GLFW_INC_PATH ${GLFW_LIB_NAME}/include)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${GLFW_LIB_NAME})

set(GLM_LIB_NAME "glm")
set(GLM_INC_PATH ${GLM_LIB_NAME})
add_library(${GLM_LIB_NAME} INTERFACE)
target_include_directories(${GLM_LIB_NAME} INTERFACE ${GLM_INC_PATH})

set(GLAD_LIB_NAME "glad")
set(GLAD_SRC_PATH "${GLAD_LIB_NAME}/src")
set(GLAD_INC_PATH "${GLAD_LIB_NAME}/include")
add_library( ${GLAD_LIB_NAME} STATIC "${GLAD_SRC_PATH}/glad.c")
target_include_directories(${GLAD_LIB_NAME} PUBLIC "${GLAD_INC_PATH}")

set(ASSIMP_LIB_NAME assimp)
set(ASSIMP_INC_PATH ${ASSIMP_LIB_NAME}/include)
add_subdirectory(${ASSIMP_LIB_NAME})