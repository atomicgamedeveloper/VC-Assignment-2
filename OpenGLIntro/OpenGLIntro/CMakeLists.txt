# CMake entry point
cmake_minimum_required(VERSION 3.1)

include(CMakePrintHelpers)
project(VC_IntroOpenGL)

cmake_print_variables(CMAKE_PREFIX_PATH)
cmake_print_variables(CMAKE_SOURCE_DIR)

# --- Dependencies ---
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(
    ${GLM_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    "external"
    ${GLFW_INCLUDE_DIRS}
    .
)

# Use experimental glm features
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

set(ALL_LIBS
    ${OPENGL_LIBRARY}
    glfw
    ${OpenCV_LIBS}
)

add_definitions(
    -DTW_STATIC
    -DTW_NO_LIB_PRAGMA
    -DTW_NO_DIRECT3D
    -DGLEW_STATIC
    -D_CRT_SECURE_NO_WARNINGS
)

# --------------------------------------------------------------------------
# Part 01 - Simple window
# --------------------------------------------------------------------------
add_executable(Part01
    Part01/simpleWindow.cpp
)
target_link_libraries(Part01
    ${ALL_LIBS}
)

# --------------------------------------------------------------------------
# Part 02 - Red triangle
# --------------------------------------------------------------------------
add_executable(Part02
    Part02/redTriangle.cpp
    common/Shader.cpp
    common/Shader.hpp
	common/ColorShader.cpp
    common/ColorShader.hpp
    common/Camera.cpp
    common/Camera.hpp
    common/Scene.cpp
    common/Scene.hpp
    common/Object.cpp
    common/Object.hpp
    common/Triangle.cpp
    common/Triangle.hpp
	common/Quad.cpp
    common/Quad.hpp
    Part02/basicShader.vert
    Part02/basicShader.frag
)
target_link_libraries(Part02
    ${ALL_LIBS}
)

# --------------------------------------------------------------------------
# Part 03 - OpenCV camera feed on textured quad
# --------------------------------------------------------------------------
add_executable(Part03
common/Shader.cpp
    common/Shader.hpp
	common/ColorShader.cpp
    common/ColorShader.hpp
    common/Camera.cpp
    common/Camera.hpp
    common/Scene.cpp
    common/Scene.hpp
    common/Object.cpp
    common/Object.hpp
    common/Triangle.cpp
    common/Triangle.hpp
	common/Texture.cpp
    common/Texture.hpp
	common/TextureShader.cpp
    common/TextureShader.hpp
	common/Quad.cpp
    common/Quad.hpp
    Part03/webcamQuad.cpp
)
target_link_libraries(Part03
    ${ALL_LIBS}
)

# --------------------------------------------------------------------------
# Source grouping for IDE organization
# --------------------------------------------------------------------------
SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*")
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$")
